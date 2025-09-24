/******************************************************************************/
/* app.c - Wi-Fi + ICM40627 + HTTP server example (SIWG917 using sl_http_server) */
/******************************************************************************/

#include "sl_board_configuration.h"
#include "cmsis_os2.h"
#include "sl_si91x_driver.h"
#include "sl_net_ping.h"
#include "sl_utility.h"
#include "sl_wifi.h"
#include "sl_net.h"
#include "sl_si91x_socket.h"
#include "sl_http_server.h"
#include "sl_net_wifi_types.h"
#include "sl_net_default_values.h"
#include <string.h>
#include <stdio.h>
#include "gyroheader.h"
#include "sl_si91x_button.h"
#include "sl_si91x_button_pin_config.h"
#include "sl_si91x_button_instances.h"



#define BACKEND_IP   "192.168.0.104"
#define BACKEND_PORT 5000
#define SEND_RETRIES 3
#define SEND_DELAY_MS 500



/********************* Thread Attributes ************************************/
const osThreadAttr_t wifi_thread_attributes = {
  .name       = "wifi_app",
  .stack_size = 3072,
  .priority   = osPriorityLow,
};

const osThreadAttr_t icm_thread_attributes = {
  .name       = "icm_app",
  .stack_size = 1024,
  .priority   = osPriorityLow,
};
const osThreadAttr_t thread_attributes = {
  .name       = "app",
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = 3072,
  .priority   = osPriorityLow,
  .tz_module  = 0,
  .reserved   = 0,
};


/********************* Wi-Fi configuration **********************************/
static const sl_wifi_device_configuration_t wifi_configuration = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .boot_config = {
    .oper_mode                  = SL_SI91X_CLIENT_MODE,
    .coex_mode                  = SL_SI91X_WLAN_ONLY_MODE,
    .feature_bit_map            = SL_SI91X_FEAT_SECURITY_OPEN,
    .tcp_ip_feature_bit_map     = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT),
    .custom_feature_bit_map     = SL_SI91X_CUSTOM_FEAT_EXTENTION_VALID,
    .ext_custom_feature_bit_map = SL_SI91X_EXT_FEAT_XTAL_CLK,
  },
};

uint32_t seconds = 0;
int8_t   button0 = BUTTON_STATE_INVALID;
int8_t   button1 = BUTTON_STATE_INVALID;

/********************* Forward declarations *********************************/
static void wifi_thread_func(void *arg);
static void icm_thread_func(void *arg);
static sl_status_t http_sensor_callback(sl_http_server_t *handle,
                                        sl_http_server_request_t *request);
void send_to_backend(const char* json_data);

/********************* Wi-Fi + HTTP server thread ***************************/
static sl_status_t http_index_callback(sl_http_server_t *handle,
                                       sl_http_server_request_t *request)
{
  (void)request;
  sl_http_server_response_t response;
  const char *html_page =
  "<!DOCTYPE html>"
  "<html lang='en'>"
  "<head>"
  "<meta charset='UTF-8'>"
  "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
  "<title>Smart Wearable Sensor Dashboard</title>"
  "<style>"
  "body { font-family:'Segoe UI',Tahoma,Geneva,Verdana,sans-serif; background:#f0f8ff; margin:0; padding:20px; }"
  "h1 { text-align:center; color:#004d40; margin-bottom:20px; text-shadow:1px 1px 2px #a7ffeb; }"
  ".container { display:flex; flex-wrap:wrap; justify-content:center; gap:20px; }"
  ".left-panel { flex:1 1 600px; display:grid; grid-template-columns: repeat(auto-fit, minmax(180px,1fr)); gap:20px; }"
  ".right-panel { flex:1 1 300px; display:flex; flex-direction:column; gap:15px; padding:10px; }"
  ".summary-card { background:#c8e6c9; padding:15px; border-radius:12px; text-align:center; font-weight:bold; box-shadow:0 4px 10px rgba(0,0,0,0.2); }"
  ".card { background:#c8e6c9; padding:15px; border-radius:12px; display:flex; flex-direction:column; align-items:center; box-shadow:0 6px 15px rgba(0,0,0,0.2); transition:all 0.5s ease; }"
  ".card.alert { animation: alertPulse 1s infinite; }"
  "@keyframes alertPulse { 0%{box-shadow:0 0 5px red;} 50%{box-shadow:0 0 20px red;} 100%{box-shadow:0 0 5px red;} }"
  ".value-status { display:flex; justify-content:space-between; width:100%; }"
  ".value { font-size:1.5em; color:#00695c; }"
  ".status { font-weight:bold; }"
  ".bar-bg { background:#eee; width:100%; height:10px; border-radius:5px; margin-top:5px; }"
  ".bar-fill { height:10px; border-radius:5px; transition: width 0.5s ease; }"
  "canvas { width:100%; height:50px; margin-top:5px; }"
  "img { display:block; margin:20px auto; width:200px; border-radius:10px; box-shadow:0 4px 10px rgba(0,0,0,0.2); transition: transform 0.5s ease; }"
  "img:hover { transform:scale(1.05); }"
  "</style>"
  "</head>"
  "<body>"
  "<h1>Smart Wearable Sensor Dashboard</h1>"
  "<div class='container'>"
  "<div class='left-panel' id='data'></div>"
  "<div class='right-panel' id='summary'></div>"
  "</div>"
  "<script>"
  // Sound notification
  "function beep(){ const a=new AudioContext(); const o=a.createOscillator(); const g=a.createGain(); o.connect(g); g.connect(a.destination); o.frequency.value=440; o.start(); g.gain.exponentialRampToValueAtTime(0.0001,a.currentTime+0.2); o.stop(a.currentTime+0.2); }"
  "function getStatus(value,type){"
  "if(type=='accel') return Math.abs(value)>1?'High Motion':'Normal';"
  "if(type=='gyro') return Math.abs(value)>50?'High Rotation':'Normal';"
  "if(type=='temp') return value>35?'Hot':value<18?'Cold':'Normal';"
  "if(type=='humidity') return value<30?'Low':value>80?'High':'Normal';"
  "if(type=='button') return value==1?'Pressed':'Released'; return ''; }"
  "let history={};"
  "async function fetchHumidity(){"
  "try{ const res=await fetch('https://api.openweathermap.org/data/2.5/weather?lat=10.818&lon=77.018&appid=2f44c8016da4214c6c215b85b9cb6e51&units=metric');"
  "const data=await res.json(); const el=document.getElementById('humidity'); if(el) el.innerText=data.main.humidity+'%'; return data.main.humidity;}catch(e){console.error(e); return '--';} }"
  "async function fetchData(){"
  "const res=await fetch('/sensor'); const json=await res.json(); const humidity=await fetchHumidity();"
  "let html=''; let alerts=0; let maxTemp=0; let maxMotion=0;"
  "const sensors=["
  "{label:'⚡ Accel X', value:json.ax, type:'accel'},"
  "{label:'⚡ Accel Y', value:json.ay, type:'accel'},"
  "{label:'⚡ Accel Z', value:json.az, type:'accel'},"
  "{label:'🌀 Gyro X', value:json.gx, type:'gyro'},"
  "{label:'🌀 Gyro Y', value:json.gy, type:'gyro'},"
  "{label:'🌀 Gyro Z', value:json.gz, type:'gyro'},"
  "{label:'🌡 Temp', value:json.t, type:'temp'},"
  "{label:'💧 Humidity', value:humidity, type:'humidity'},"
  "{label:'⏱ Seconds', value:json.sec, type:''},"
  "{label:'🔘 Button0', value:json.b0, type:'button'},"
  "{label:'🔘 Button1', value:json.b1, type:'button'}"
  "];"
  // Left panel cards
  "sensors.forEach(s=>{"
  "if(!history[s.label]) history[s.label]=[]; history[s.label].push(s.value); if(history[s.label].length>10) history[s.label].shift();"
  "let status=getStatus(s.value,s.type);"
  "let bg='#c8e6c9'; let color='#00695c'; let barColor='#00695c'; let alertClass='';"
  "if(s.type=='accel'&&Math.abs(s.value)>1){ bg='#ffcdd2'; color='#b71c1c'; barColor='#b71c1c'; alertClass='alert'; alerts++; maxMotion=Math.max(maxMotion,Math.abs(s.value)); }"
  "else if(s.type=='gyro'&&Math.abs(s.value)>50){ bg='#ffe0b2'; color='#e65100'; barColor='#e65100'; alertClass='alert'; alerts++; maxMotion=Math.max(maxMotion,Math.abs(s.value)); }"
  "else if(s.type=='temp'&&(s.value>35||s.value<18)){ bg='#ffcc80'; color='#bf360c'; barColor='#bf360c'; alertClass='alert'; alerts++; maxTemp=Math.max(maxTemp,s.value); }"
  "else if(s.type=='humidity'&&(s.value<30||s.value>80)){ bg='#b3e5fc'; color='#01579b'; barColor='#01579b'; alertClass='alert'; alerts++; }"
  "html+=`<div class='card ${alertClass}' style='background:${bg};color:${color}'>"
  "<div class='value-status'><div class='value'>${s.value}</div><div class='status'>${status}</div></div>"
  "<div class='bar-bg'><div class='bar-fill' style='width:${Math.min(Math.abs(s.value)*10,100)}%;background:${barColor}'></div></div>"
  "<canvas id='chart-${s.label}'></canvas></div>`;"
  "});"
  "document.getElementById('data').innerHTML=html;"
  // Draw sparklines
  "sensors.forEach(s=>{ const c=document.getElementById('chart-'+s.label); if(c){ const ctx=c.getContext('2d'); ctx.clearRect(0,0,c.width,c.height); ctx.beginPath(); let len=history[s.label].length; history[s.label].forEach((v,i)=>{ let x=i*(c.width/10); let y=c.height-(v/100)*c.height; if(i==0) ctx.moveTo(x,y); else ctx.lineTo(x,y); }); ctx.strokeStyle='#004d40'; ctx.lineWidth=2; ctx.stroke(); } });"
  // Summary
  "let summaryHTML=`<div class='summary-card'>Active Alerts: ${alerts}</div>`;"
  "summaryHTML+=`<div class='summary-card'>Max Temp: ${maxTemp}</div>`;"
  "summaryHTML+=`<div class='summary-card'>Max Motion/Rotation: ${maxMotion}</div>`;"
  "document.getElementById('summary').innerHTML=summaryHTML;"
  // Dynamic image
  "const img=document.getElementById('projectImage');"
  "if(maxTemp>35) img.src='D:/SiliconLabs/photo_hot.jpg';"
  "else if(maxMotion>1) img.src='file:///D:/SiliconLabs/photo_motion.jpg';"
  "else img.src='file:///D:/SiliconLabs/photo.jpg';"
  // Sound alert
  "if(alerts>0) beep();"
  "}"
  "setInterval(fetchData,1000); fetchData();"
  "</script>"
  "</body>"
  "</html>";

  response.response_code = SL_HTTP_RESPONSE_OK;
  response.content_type = SL_HTTP_CONTENT_TYPE_TEXT_HTML;
  response.data = (uint8_t *)html_page;
  response.current_data_length = strlen(html_page);
  response.expected_data_length = strlen(html_page);
  response.headers = NULL;
  response.header_count = 0;

  return sl_http_server_send_response(handle, &response);
}

static void wifi_thread_func(void *arg)
{
  (void)arg;
  sl_status_t status;

  // Initialize Wi-Fi
  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &wifi_configuration, NULL, NULL);
  if (status != SL_STATUS_OK) {
    printf("Wi-Fi init failed: 0x%lx\r\n", status);
    return;
  }
  printf("Wi-Fi Init Success\r\n");

  status = sl_net_up(SL_NET_WIFI_CLIENT_INTERFACE, 0);
  if (status != SL_STATUS_OK) {
    printf("Wi-Fi up failed: 0x%lx\r\n", status);
    return;
  }
  printf("Wi-Fi Connected\r\n");

  // Configure HTTP server
  sl_http_server_t server;
  sl_http_server_handler_t handlers[] = {
      { "/", http_index_callback },
      { "/sensor", http_sensor_callback }
  };


  sl_http_server_config_t config = {0};
  config.port = 80;
  config.handlers_list = handlers;
  config.handlers_count = sizeof(handlers)/sizeof(handlers[0]);
  config.default_handler = NULL;
  config.client_idle_time = 30;

  status = sl_http_server_init(&server, &config);
  if (status != SL_STATUS_OK) {
    printf("HTTP server init failed: 0x%lx\r\n", status);
    return;
  }

  status = sl_http_server_start(&server);
  if (status != SL_STATUS_OK) {
    printf("HTTP server start failed: 0x%lx\r\n", status);
    return;
  }

  printf("HTTP Server Running on port 80\r\n");

  while (1) {
    osDelay(1000); // keep thread alive
    seconds++;
    button0 = sl_si91x_button_pin_state(SL_BUTTON_BTN0_PIN);
    button1 = sl_si91x_button_pin_state(SL_BUTTON_BTN1_PIN);
  }
}

/********************* HTTP /sensor callback *******************************/
static sl_status_t http_sensor_callback(sl_http_server_t *handle,
                                        sl_http_server_request_t *request)
{
  (void)request; // unused for GET
  sl_http_server_response_t response;
  float accel[3], gyro[3], temp;

  // Read sensor data
  icm40627_example_get_accel(accel);
  icm40627_example_get_gyro(gyro);
  icm40627_example_get_temperature(&temp);

  // Prepare JSON response
  // Prepare JSON same as in /sensor callback
  char json_buf[256];
  snprintf(json_buf, sizeof(json_buf),
           "{\"ax\":%.2f,\"ay\":%.2f,\"az\":%.2f,"
           "\"gx\":%.2f,\"gy\":%.2f,\"gz\":%.2f,"
           "\"t\":%.2f,"
           "\"sec\":%lu,"
           "\"b0\":%d,"
           "\"b1\":%d}",
           accel[0], accel[1], accel[2],
           gyro[0], gyro[1], gyro[2],
           temp,
           seconds,
           button0,
           button1);

  // Send to backend
  //send_to_backend(json_buf);


    // Fill response structure
    response.response_code = SL_HTTP_RESPONSE_OK;
    response.content_type = SL_HTTP_CONTENT_TYPE_APPLICATION_JSON;
    response.data = (uint8_t *)json_buf;
    response.current_data_length = strlen(json_buf);
    response.expected_data_length = strlen(json_buf);
    response.headers = NULL;
    response.header_count = 0;
  // Send response
  return sl_http_server_send_response(handle, &response);
}
/*void send_to_backend(const char *json_data)
{
    int32_t sock;
    struct sockaddr_in server_addr = {0};
    char request[512];
    int32_t ret;
    int attempt;
    uint32_t ip_addr = 0;

    if (json_data == NULL || strlen(json_data) == 0) {
        return; // nothing to send
    }

    // Convert IP string to binary
    ret = sl_net_inet_addr(BACKEND_IP, &ip_addr);
    if (ret != SL_STATUS_OK) {
        printf("Invalid backend IP\r\n");
        return;
    }

    // Fill server address
        server_addr.sin_family = AF_INET;
        server_addr.sin_port   = htons(BACKEND_PORT);                 // convert to network byte order
        server_addr.sin_addr.s_addr = sl_net_inet_addr(BACKEND_IP);   // convert string IP to hex

    // Format HTTP POST request
    snprintf(request, sizeof(request),
             "POST /sensor_data HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %d\r\n"
             "\r\n"
             "%s",
             BACKEND_IP,
             (int)strlen(json_data),
             json_data);

    // Retry logic
    for (attempt = 0; attempt < SEND_RETRIES; attempt++)
    {
        // Create socket
        sock = sl_si91x_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock < 0) {
            printf("Socket creation failed (attempt %d)\r\n", attempt + 1);
            osDelay(SEND_DELAY_MS);
            continue;
        }

        // Connect to backend
        ret = sl_si91x_connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
        if (ret != SL_STATUS_OK) {
            printf("Connect failed (attempt %d): 0x%ld\r\n", attempt + 1, (long)ret);
            sl_si91x_close(sock);
            osDelay(SEND_DELAY_MS);
            continue;
        }

        // Send HTTP request
        ret = sl_si91x_send(sock, (const uint8_t *)request, strlen(request), 0);
        if (ret < 0) {
            printf("Send failed (attempt %d): 0x%ld\r\n", attempt + 1, (long)ret);
        } else {
            printf("Data sent successfully (attempt %d)\r\n", attempt + 1);
        }

        // Close socket
        sl_si91x_close(sock);
        break; // exit retry loop if sent
    }
}
*/
/********************* ICM sensor thread ***********************************/
static void icm_thread_func(void *arg)
{
  (void)arg;
  icm40627_example_init();

  while (1) {
    icm40627_example_process_action();
    osDelay(2000);
  }
}

/******************************************************************************/
// Application init - create threads
void app_init(void)
{
  osThreadNew(wifi_thread_func, NULL, &wifi_thread_attributes);
  osThreadNew(icm_thread_func, NULL, &icm_thread_attributes);
}

/******************************************************************************/
// Called when no RTOS present (kept for compatibility)
void app_process_action(void)
{
  // nothing (RTOS used)
}
