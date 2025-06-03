#include <WiFi.h>
  #include <WiFiClient.h>
  #include <WebServer.h>
  #include <DHT.h>
// include <ESPmDNS.h>
  #include <SPIFFS.h>
  #include "esp_task_wdt.h"
  
  // *WiFi Credentials*
  const char *ssid = "Infinix HOT 40i";  
  const char *password = "maham480";
  
  // *Sensor Configuration*
  #define DHTPIN 26
  #define DHTTYPE DHT22
  DHT dht(DHTPIN, DHTTYPE);
  
  WebServer server(80);
  const char *userFile = "/users.txt";  // User storage file
  
  // *Initialize SPIFFS*
  void initSPIFFS() {
      if (!SPIFFS.begin(true)) {
          Serial.println("⚠ SPIFFS initialization failed! Formatting...");
          SPIFFS.format();
          if (!SPIFFS.begin(true)) {
              Serial.println("❌ SPIFFS failed even after formatting!");
          } else {
              Serial.println("✅ SPIFFS Reformatted and Mounted Successfully.");
          }
      } else {
          Serial.println("✅ SPIFFS Initialized.");
      }
  }
  
  // *Fix: Properly Handle WiFi Connection*
  void connectToWiFi() {
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid, password);
      Serial.print("Connecting to WiFi");
  
      int attempts = 0;
      while (WiFi.status() != WL_CONNECTED && attempts < 30) {
          delay(500);
          Serial.print(".");
          attempts++;
      }
  
      if (WiFi.status() == WL_CONNECTED) {
          Serial.println("\n✅ WiFi Connected!");
          Serial.print("📶 IP Address: ");
          Serial.println(WiFi.localIP());
      } else {
          Serial.println("\n❌ WiFi Connection Failed! Check SSID/Password.");
      }
  }
  
  // *Fix: Disable Watchdog Timer*
  void disableWatchdog() {
      Serial.println("🛠 Disabling Watchdog Timer...");
      esp_task_wdt_delete(NULL);
  }
  
  // *Fix: Save User Data Properly (Only Username & Password)*
  void saveUser(String username, String password) {
      username.trim();
      password.trim();
  
      // ✅ Fix: Call replace() on separate lines
      username.replace("\r", "");
      username.replace("\n", "");
      username.replace("\t", "");
      
      password.replace("\r", "");
      password.replace("\n", "");
      password.replace("\t", "");
  
      File file = SPIFFS.open(userFile, FILE_APPEND);
      if (!file) {
          Serial.println("❌ Failed to open user file for writing.");
          return;
      }
  
      file.println(username + "," + password);  // ✅ Fix: No extra space after comma
      file.close();
      
      Serial.println("✅ User Registered: [" + username + "] [" + password + "]");
      printUserFile();
  }
  
  
  
  // *Fix: Validate User Credentials Properly*
  
  bool validateUser(String username, String password) {
      username.trim();
      password.trim();
  
      File file = SPIFFS.open(userFile, FILE_READ);
      if (!file) {
          Serial.println("❌ Failed to open user file.");
          return false;
      }
  
      Serial.println("🔍 Checking Login for: [" + username + "] | [" + password + "]");
  
      while (file.available()) {
          String line = file.readStringUntil('\n');
  
          line.replace("\r", "");
          line.replace("\n", "");
          line.replace("\t", "");
          line.trim();
  
          int commaIndex = line.indexOf(',');
          if (commaIndex == -1) continue;
  
          String storedUser = line.substring(0, commaIndex);
          String storedPass = line.substring(commaIndex + 1);
  
          storedUser.trim();
          storedPass.trim();
  
          Serial.println("📌 Stored User: [" + storedUser + "] vs Entered User: [" + username + "]");
          Serial.println("📌 Stored Pass: [" + storedPass + "] vs Entered Pass: [" + password + "]");
  
          if (storedUser.equals(username) && storedPass.equals(password)) {  // Use .equals() instead of ==
              file.close();
              Serial.println("✅ Login Successful!");
              return true;
          }
      }
  
      file.close();
      Serial.println("❌ Invalid Login (No Match Found)");
      return false;
  }
  
  void printUserFile() {
      File file = SPIFFS.open(userFile, FILE_READ);
      if (!file) {
          Serial.println("❌ Failed to open user file for reading.");
          return;
      }
      Serial.println("📂 Registered Users in SPIFFS:");
  
      while (file.available()) {
          String line = file.readStringUntil('\n');
          
          // ✅ Remove all unwanted characters
          line.replace("\r", "");
          line.replace("\n", "");
          line.replace("\t", "");
          line.trim();
  
          Serial.println("📌 [" + line + "]"); // Print each user entry with brackets
      }
  
      file.close();
  }
  
  
  
  void listAllUsers() {
      File file = SPIFFS.open(userFile, FILE_READ);
      if (!file) {
          Serial.println("❌ Failed to open user file.");
          return;
      }
  
      Serial.println("📂 Registered Users:");
      while (file.available()) {
          Serial.println(file.readStringUntil('\n')); // File ka data print kare
      }
  
      file.close();
  }
  
  
  
  // *Signup Page*
  const char signUpPage[] PROGMEM = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
      <title>Sign Up</title>
      <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css">
  </head>
  <body class="bg-dark text-white text-center">
      <div class="container mt-5">
          <h2 class="mb-4">Create Account</h2>
          <div class="card p-4 bg-light text-dark">
              <input type="text" id="username" class="form-control mb-3" placeholder="Enter Username">
              <input type="password" id="password" class="form-control mb-3" placeholder="Enter Password">
              <button class="btn btn-primary" onclick="signup()">Sign Up</button>
              <p class="mt-3">Already have an account? <a href='/'>Login</a></p>
          </div>
      </div>
      <script>
          function signup() {
              let user = document.getElementById('username').value;
              let pass = document.getElementById('password').value;
  
              fetch('/signup', {
                  method: 'POST',
                  headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                  body: 'username=' + user + '&password=' + pass
              }).then(response => response.text()).then(result => {
                  if (result == 'success') {
                      alert('Account Created! Please log in.');
                      window.location.href = "/";
                  } else {
                      alert('Signup failed.');
                  }
              });
          }
      </script>
  </body>
  </html>
  )rawliteral";
  
  // *Fix: Handle Sign-Up Properly*
  void handleSignUp() {
      if (server.method() == HTTP_POST) {
          String username = server.arg("username");
          String password = server.arg("password");
  
          username.trim();  // Remove extra spaces
          password.trim();
  
          Serial.println("🔹 Storing User: " + username);
          Serial.println("🔹 Storing Pass: " + password);
  
          saveUser(username, password);  // Save to file
          server.send(200, "text/plain", "success");
      } else {
          server.send(200, "text/html", signUpPage);
      }
  }
  
  
  // *Login Page (Restored the Missing GUI)*
  const char loginPage[] PROGMEM = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
      <title>Login</title>
      <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css">
  </head>
  <body class="bg-dark text-white text-center">
      <div class="container mt-5">
          <h2 class="mb-4">Login</h2>
          <div class="card p-4 bg-light text-dark">
              <input type="text" id="username" class="form-control mb-3" placeholder="Enter Username">
              <input type="password" id="password" class="form-control mb-3" placeholder="Enter Password">
              <button class="btn btn-success" onclick="login()">Login</button>
              <p class="mt-3">Don't have an account? <a href='/signup'>Sign Up</a></p>
          </div>
      </div>
      <script>
      function login() {
      let user = document.getElementById('username').value.trim();
      let pass = document.getElementById('password').value.trim();
  
      fetch('/login', {
          method: 'POST',
          headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
          body: 'username=' + encodeURIComponent(user) + '&password=' + encodeURIComponent(pass)
      })
      .then(response => response.text())
      .then(result => {
          console.log("Server Response:", result);  // ✅ Debugging: Print server response
          if (result.trim() === 'success') {
              window.location.href = "/dashboard";
          } else {
              alert('Invalid login. Please check username and password.');
          }
      })
      .catch(error => {
          console.error('Error:', error);
          alert('Network error. Try again.');
      });
  }
  
  </script>
  
  </body>
  </html>
  )rawliteral";
  
  const char dashboardPage[] PROGMEM = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
      <title>Dashboard</title>
      <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css">
      <style>
          body {
              background-color: #f8f9fa; /* Light grayish background */
              color: #333;
              text-align: center;
              font-family: Arial, sans-serif;
          }
  
          .container {
              margin-top: 50px;
              background: white;
              padding: 30px;
              border-radius: 10px;
              box-shadow: 2px 4px 10px rgba(0, 0, 0, 0.1);
          }
  
          .btn-custom {
              width: 80%;
              font-size: 18px;
              padding: 12px;
              margin: 10px auto;
              display: block;
              border-radius: 8px;
              border: none;
          }
  
          .btn-danger { background-color: #ff6b6b; color: white; } /* Soft Red */
          .btn-info { background-color: #4db8ff; color: white; }  /* Soft Blue */
          .btn-secondary { background-color: #b8b8b8; color: white; } /* Soft Gray */
  
          .btn-danger:hover { background-color: #ff5252; }
          .btn-info:hover { background-color: #3399ff; }
          .btn-secondary:hover { background-color: #9e9e9e; }
      </style>
  </head>
  <body>
      <div class="container">
          <h2 class="mb-4"><i class="fas fa-user"></i> Select Profile </h2>
          <button class="btn btn-danger btn-custom" onclick="selectProfile('Asthmatic')">Asthmatic Profile</button>
          <button class="btn btn-info btn-custom" onclick="selectProfile('NonAsthmatic')">Non-Asthmatic Profile</button>
           <button class="btn btn-secondary btn-custom" onclick="goBack()">Back</button>
      </div>
  
      <script>
          function selectProfile(type) {
              if (type === "Asthmatic") {
                  window.location.href = "/asthmatic-dashboard";
              } else {
                  window.location.href = "/non-asthmatic-dashboard";
              }
          }
  
          function goBack() {
              window.location.href = "/";
          }
      </script>
  </body>
  </html>
  )rawliteral";
  
 const char asthmaticDashboardPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Asthma Monitoring Dashboard</title>
    
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0/css/all.min.css">
    <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css">

    <style>
        body {
            background-color: #f0f8ff;
            color: #333;
            font-family: Arial, sans-serif;
            text-align: center;
        }

        .container {
            margin-top: 30px;
        }

        .dashboard-section {
            background: white;
            border-radius: 10px;
            box-shadow: 0px 4px 8px rgba(0, 0, 0, 0.1);
            padding: 20px;
            margin: 15px;
        }

        .card {
            background: #fdfdfd;
            padding: 15px;
            border-radius: 10px;
            margin-bottom: 15px;
            display: flex;
            align-items: center;
            justify-content: space-between;
            box-shadow: 2px 2px 8px rgba(0, 0, 0, 0.1);
        }

        .icon {
            font-size: 30px;
            width: 40px;
            color: #007bff;
        }

        .sensor-text {
            flex-grow: 1;
            text-align: left;
            font-size: 18px;
            font-weight: bold;
        }

        .sensor-value {
            font-size: 22px;
            font-weight: bold;
            color: #0056b3;
        }

        .btn-custom {
            width: 100%;
            margin-top: 15px;
            font-size: 18px;
            background: #007bff;
            color: white;
        }

        .btn-custom:hover {
            background: #0056b3;
        }

        .durationOptions {
            display: none;
            margin-top: 15px;
        }

        .small-btn {
            width: 30%;
            margin: 5px;
            font-size: 16px;
        }

        .row {
            justify-content: center;
        }

        h2 {
            margin-bottom: 25px;
        }

        .section-header {
            font-weight: bold;
            margin-bottom: 15px;
            font-size: 20px;
            color: #007bff;
        }
    </style>
</head>

<body>
  <div class="container">
    <h2><i class="fas fa-lungs"></i> Asthma Monitoring Dashboard</h2>
    
    <div class="row">
        <!-- Node 1 -->
        <div class="col-md-6">
            <h4>Node 1</h4>

            <div class="card">
                <i class="fas fa-cloud icon"></i>
                <div class="sensor-text">CO2 (ppm)</div>
                <div class="sensor-value" id="co2Value1">--</div>
            </div>

            <div class="card">
                <i class="fas fa-wind icon"></i>
                <div class="sensor-text">Dust (µg/m³)</div>
                <div class="sensor-value" id="dustValue1">--</div>
            </div>

            <div class="card">
                <i class="fas fa-thermometer-half icon"></i>
                <div class="sensor-text">Temperature (°C)</div>
                <div class="sensor-value" id="tempValue1">--</div>
            </div>

            <div class="card">
                <i class="fas fa-tint icon"></i>
                <div class="sensor-text">Humidity (%)</div>
                <div class="sensor-value" id="humidityValue1">--</div>
            </div>

            <button class="btn btn-custom" onclick="showDurationOptions(1)">Turn On Purifier</button>
            <div id="durationOptions1" class="durationOptions">
                <p>Select Duration:</p>
                <button class="btn btn-primary small-btn" onclick="turnOnPurifier(1, 10)">10 seconds</button>
                <button class="btn btn-primary small-btn" onclick="turnOnPurifier(1, 20)">20 seconds</button>
                <button class="btn btn-primary small-btn" onclick="turnOnPurifier(1, 30)">30 seconds</button>
            </div>
        </div>

        <!-- Node 2 -->
        <div class="col-md-6">
            <h4>Node 2</h4>

            <div class="card">
                <i class="fas fa-cloud icon"></i>
                <div class="sensor-text">CO2 (ppm)</div>
                <div class="sensor-value" id="co2Value2">--</div>
            </div>

            <div class="card">
                <i class="fas fa-wind icon"></i>
                <div class="sensor-text">Dust (µg/m³)</div>
                <div class="sensor-value" id="dustValue2">--</div>
            </div>

            <div class="card">
                <i class="fas fa-thermometer-half icon"></i>
                <div class="sensor-text">Temperature (°C)</div>
                <div class="sensor-value" id="tempValue2">--</div>
            </div>

            <div class="card">
                <i class="fas fa-tint icon"></i>
                <div class="sensor-text">Humidity (%)</div>
                <div class="sensor-value" id="humidityValue2">--</div>
            </div>

            <button class="btn btn-custom" onclick="showDurationOptions(2)">Turn On Purifier</button>
            <div id="durationOptions2" class="durationOptions">
                <p>Select Duration:</p>
                <button class="btn btn-primary small-btn" onclick="turnOnPurifier(2, 10)">10 seconds</button>
                <button class="btn btn-primary small-btn" onclick="turnOnPurifier(2, 20)">20 seconds</button>
                <button class="btn btn-primary small-btn" onclick="turnOnPurifier(2, 30)">30 seconds</button>
            </div>
        </div>
    </div>

    <button class="btn btn-light btn-custom mt-3" onclick="goBack()">Back</button>
</div>
<script>
    // Fake sensor data arrays
    const fakeData = {
        co2_1: [195, 168, 155, 143, 138, 133, 128, 124],
        dust_1: [22.0, 21.5, 21.4, 21.2, 21.0, 20.9, 21.1, 21.3],
        temp_1: [31.5, 31, 31, 31, 31, 31, 31, 31],
        humidity_1: [90, 52, 52, 55, 60, 52, 52, 50],

        co2_2: [188, 165, 159, 140, 134, 129, 126, 121],
        dust_2: [19.0, 18.7, 18.4, 18.3, 18.2, 18.1, 18.0, 18.2],
        temp_2: [30.5, 30, 30, 30, 30, 30, 30, 30],
        humidity_2: [85, 60, 58, 57, 55, 53, 52, 50]
    };

    let index = 0;

    function updateFakeSensorData() {
        index = (index + 1) % fakeData.co2_1.length;

        document.getElementById('co2Value1').innerText = fakeData.co2_1[index] + " ppm";
        document.getElementById('dustValue1').innerText = fakeData.dust_1[index].toFixed(1) + " µg/m³";
        document.getElementById('tempValue1').innerText = fakeData.temp_1[index].toFixed(1) + " °C";
        document.getElementById('humidityValue1').innerText = fakeData.humidity_1[index] + " %";

        document.getElementById('co2Value2').innerText = fakeData.co2_2[index] + " ppm";
        document.getElementById('dustValue2').innerText = fakeData.dust_2[index].toFixed(1) + " µg/m³";
        document.getElementById('tempValue2').innerText = fakeData.temp_2[index].toFixed(1) + " °C";
        document.getElementById('humidityValue2').innerText = fakeData.humidity_2[index] + " %";
    }

    function showDurationOptions(node) {
        document.getElementById("durationOptions" + node).style.display = "block";
    }

    function turnOnPurifier(node, duration) {
        alert("Purifier for Node " + node + " will run for " + duration + " seconds.");
        fetch(/turn-purifier?node=${node}&duration=${duration})
            .then(response => response.text())
            .then(result => console.log(result))
            .catch(err => console.error("Error turning on purifier:", err));
    }

    function goBack() {
        window.location.href = "/dashboard";
    }

    // Call every 3 seconds
    setInterval(updateFakeSensorData, 3000);

    // Initial call on page load
    window.onload = updateFakeSensorData;
</script>


//    <script>
//        function fetchSensorData() {
//            fetch('/sensor-data')
//                .then(response => response.json())
//                .then(data => {
//                    document.getElementById('co2Value1').innerText = data.node1.co2 + " ppm";
//                    document.getElementById('dustValue1').innerText = data.node1.dust + " µg/m³";
//                    document.getElementById('tempValue1').innerText = data.node1.temp + " °C";
//                    document.getElementById('humidityValue1').innerText = data.node1.humidity + " %RH";
//
//                    document.getElementById('co2Value2').innerText = data.node2.co2 + " ppm";
////                    document.getElementById('vocValue2').innerText = data.node2.voc + " ppm";
//                    document.getElementById('dustValue2').innerText = data.node2.dust + " µg/m³";
//                    document.getElementById('tempValue2').innerText = data.node2.temp + " °C";
//                    document.getElementById('humidityValue2').innerText = data.node2.humidity + " %RH";
//                })
//                .catch(error => console.error('Error fetching sensor data:', error));
//        }
//
//        function showDurationOptions(node) {
//            document.getElementById("durationOptions" + node).style.display = "block";
//        }
//
//        function turnOnPurifier(duration, node) {
//            alert("Purifier for Node " + node + " will run for " + duration + " minutes.");
//            // Backend request to /purifier?node=node&duration=minutes (if needed)
//        }
//
//        function goBack() {
//            window.location.href = "/dashboard";
//        }
//
//        setInterval(fetchSensorData, 5000); // Refresh every 5 seconds
//    </script>
</body>
</html>
)rawliteral";

  
const char nonasthmaticDashboardPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Non-Asthmatic Dashboard</title>
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0/css/all.min.css">
  <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css">
  <style>
    body {
      background-color: #f0f8ff;
      color: #333;
      font-family: Arial, sans-serif;
      text-align: center;
    }
    .container {
      background: white;
      border-radius: 10px;
      box-shadow: 0px 4px 8px rgba(0, 0, 0, 0.1);
      padding: 20px;
      margin-top: 30px;
    }
    .card {
      background: #fdfdfd;
      padding: 15px;
      border-radius: 10px;
      margin-bottom: 15px;
      display: flex;
      align-items: center;
      justify-content: space-between;
      box-shadow: 2px 2px 8px rgba(0, 0, 0, 0.1);
    }
    .icon {
      font-size: 30px;
      width: 40px;
      color: #007bff;
    }
    .sensor-text {
      flex-grow: 1;
      text-align: left;
      font-size: 18px;
      font-weight: bold;
    }
    .sensor-value {
      font-size: 22px;
      font-weight: bold;
      color: #0056b3;
    }
    .btn-custom {
      width: 100%;
      margin-top: 10px;
      font-size: 18px;
      background: #007bff;
      color: white;
    }
    .btn-custom:hover {
      background: #0056b3;
    }
    h4 {
      margin-top: 15px;
      font-weight: bold;
    }
  </style>
</head>
<body>
  <div class="container">
    <h2><i class="fas fa-lungs"></i> Non-Asthma Monitoring Dashboard</h2>
    <div class="row justify-content-center">
      <!-- Node 1 -->
      <div class="col-md-6">
        <h4>Node 1</h4>
        <div class="card"><i class="fas fa-cloud icon"></i><div class="sensor-text">CO2 (ppm)</div><div class="sensor-value" id="node1_co2">--</div></div>
        <div class="card"><i class="fas fa-wind icon"></i><div class="sensor-text">Dust (µg/m³)</div><div class="sensor-value" id="node1_dust">--</div></div>
        <div class="card"><i class="fas fa-thermometer-half icon"></i><div class="sensor-text">Temperature (°C)</div><div class="sensor-value" id="node1_temp">--</div></div>
        <div class="card"><i class="fas fa-tint icon"></i><div class="sensor-text">Humidity (%)</div><div class="sensor-value" id="node1_humidity">--</div></div>
      </div>

      <!-- Node 2 -->
      <div class="col-md-6">
        <h4>Node 2</h4>
        <div class="card"><i class="fas fa-cloud icon"></i><div class="sensor-text">CO2 (ppm)</div><div class="sensor-value" id="node2_co2">--</div></div>
        <div class="card"><i class="fas fa-wind icon"></i><div class="sensor-text">Dust (µg/m³)</div><div class="sensor-value" id="node2_dust">--</div></div>
        <div class="card"><i class="fas fa-thermometer-half icon"></i><div class="sensor-text">Temperature (°C)</div><div class="sensor-value" id="node2_temp">--</div></div>
        <div class="card"><i class="fas fa-tint icon"></i><div class="sensor-text">Humidity (%)</div><div class="sensor-value" id="node2_humidity">--</div></div>
      </div>
    </div>

    <button class="btn btn-light btn-custom" onclick="goBack()">Back</button>
  </div>
<script>
    // Fake sensor data arrays
    const fakeData = {
        co2_1: [195, 168, 155, 143, 138, 133, 128, 124],
        dust_1: [22.0, 21.5, 21.4, 21.2, 21.0, 20.9, 21.1, 21.3],
        temp_1: [31.5, 31, 31, 31, 31, 31, 31, 31],
        humidity_1: [90, 52, 52, 55, 60, 52, 52, 50],

        co2_2: [188, 165, 159, 140, 134, 129, 126, 121],
        dust_2: [19.0, 18.7, 18.4, 18.3, 18.2, 18.1, 18.0, 18.2],
        temp_2: [30.5, 30, 30, 30, 30, 30, 30, 30],
        humidity_2: [85, 60, 58, 57, 55, 53, 52, 50]
    };

    let index = 0;

    function updateFakeSensorData() {
        index = (index + 1) % fakeData.co2_1.length;

        document.getElementById('co2Value1').innerText = fakeData.co2_1[index] + " ppm";
        document.getElementById('dustValue1').innerText = fakeData.dust_1[index].toFixed(1) + " µg/m³";
        document.getElementById('tempValue1').innerText = fakeData.temp_1[index].toFixed(1) + " °C";
        document.getElementById('humidityValue1').innerText = fakeData.humidity_1[index] + " %";

        document.getElementById('co2Value2').innerText = fakeData.co2_2[index] + " ppm";
        document.getElementById('dustValue2').innerText = fakeData.dust_2[index].toFixed(1) + " µg/m³";
        document.getElementById('tempValue2').innerText = fakeData.temp_2[index].toFixed(1) + " °C";
        document.getElementById('humidityValue2').innerText = fakeData.humidity_2[index] + " %";
    }

    function showDurationOptions(node) {
        document.getElementById("durationOptions" + node).style.display = "block";
    }

    function turnOnPurifier(node, duration) {
        alert("Purifier for Node " + node + " will run for " + duration + " seconds.");
        fetch(/turn-purifier?node=${node}&duration=${duration})
            .then(response => response.text())
            .then(result => console.log(result))
            .catch(err => console.error("Error turning on purifier:", err));
    }

    function goBack() {
        window.location.href = "/dashboard";
    }

    // Call every 3 seconds
    setInterval(updateFakeSensorData, 3000);

    // Initial call on page load
    window.onload = updateFakeSensorData;
</script>

</body>
</html>
)rawliteral";


  
  // *Fix: Handle Login Properly*
  void handleLogin() {
      if (server.method() == HTTP_POST) {
          String username = server.arg("username");
          String password = server.arg("password");
  
          Serial.println("🔍 Login Attempt: " + username + " | " + password);
  
          if (validateUser(username, password)) {
              Serial.println("✅ Sending Response: success");
              server.send(200, "text/plain", "success");
          } else {
              Serial.println("❌ Sending Response: fail");
              server.send(200, "text/plain", "fail");
          }
      } else {
          server.send(200, "text/html", loginPage);
      }
  }
  
  
  void clearUserFile() {
      SPIFFS.remove(userFile);
      Serial.println("🗑 Cleared user file!");
  }
  void handleSensorData() {
      float avgCO2 = 450.0;  // Replace with actual averaging logic
      float avgVOC = 120.0;
      float avgDust = 35.5;
      float avgTemp = 25.6;
      float avgHumidity = 65.2;
  
      String jsonData = "{";
      jsonData += "\"co2\":" + String(avgCO2) + ",";
      jsonData += "\"voc\":" + String(avgVOC) + ",";
      jsonData += "\"dust\":" + String(avgDust) + ",";
      jsonData += "\"temp\":" + String(avgTemp) + ",";
      jsonData += "\"humidity\":" + String(avgHumidity);
      jsonData += "}";
  
      server.send(200, "application/json", jsonData);
  }
  
  // *Setup*
  void setup() {
      Serial.begin(115200);
      Serial.println("🚀 Booting...");
  
      initSPIFFS();
      connectToWiFi();
      disableWatchdog();
      
       printUserFile();  // ✅ Print users at startup
      server.on("/", handleLogin);
      server.on("/login", handleLogin);
      server.on("/signup", handleSignUp);
     
  server.on("/dashboard", []() { server.send(200, "text/html", dashboardPage); });
  server.on("/asthmatic-dashboard", []() { server.send(200, "text/html", asthmaticDashboardPage); });
  server.on("/non-asthmatic-dashboard", []() { server.send(200, "text/html",nonasthmaticDashboardPage); });
  
  
  
      server.begin();
      Serial.println("✅ HTTP Server Started");
  }
  
  // *Loop*
  void loop() {
      server.handleClient();
      delay(10);
  }
