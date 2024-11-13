/* By Abdullah As-Sadeed */

function Show_Alert(message) {
  if (typeof message !== "undefined") {
    if (message !== "") {
      var alert_box = document.createElement("div");
      alert_box.innerHTML = message;
      alert_box.classList.add("alert_box");
      alert_box.classList.add("show");
      document.body.append(alert_box);
      setTimeout(function () {
        alert_box.classList.remove("show");
      }, 3000);
      setTimeout(function () {
        alert_box.remove();
      }, 4000);
    } else {
      return false;
    }
  } else {
    return false;
  }
}

document.oncontextmenu = function (contextmenu) {
  contextmenu.preventDefault();
  Show_Alert("Context menu is not allowed");
  return false;
};

document.onkeydown = function (keyboard) {
  if (
    keyboard.key == "F12" ||
    (keyboard.ctrlKey && keyboard.shiftKey && keyboard.key == "i") ||
    (keyboard.ctrlKey && keyboard.shiftKey && keyboard.key == "j") ||
    (keyboard.ctrlKey && keyboard.key == "u")
  ) {
    keyboard.preventDefault();
    Show_Alert("Developer tools are not allowed");
    return false;
  } else if (
    keyboard.Code == "PrintScreen" ||
    (keyboard.ctrlKey && keyboard.key == "p")
  ) {
    keyboard.preventDefault();
    Show_Alert("Printing is not allowed");
    return false;
  } else if (keyboard.ctrlKey && keyboard.key == "s") {
    keyboard.preventDefault();
    Show_Alert("Saving is not allowed");
    return false;
  }
};

function bind_chart(canvas, line_count, show_shade) {
  if (line_count > 0) {
    const chart = new SmoothieChart({
      labels: {
        showIntermediateLabels: true,
        fillStyle: "black",
      },
      grid: {
        fillStyle: "transparent",
      },
      tooltip: true,
      responsive: true,
    });

    chart.streamTo(document.getElementById(canvas));

    var lines = [];
    var colors = ["red", "green", "blue"];
    var color_shades = [
      "rgba(255, 0, 0, 0.4)",
      "rgba(0, 255, 0, 0.4)",
      "rgba(0, 0, 255, 0.4)",
    ];

    for (i = 0; i <= line_count; i++) {
      var line = new TimeSeries();
      chart.addTimeSeries(line, {
        interpolation: "bezier",
        lineWidth: 3,
        strokeStyle: colors[i],
        fillStyle: show_shade ? color_shades[i] : undefined,
      });
      lines.push(line);
    }
    return lines;
  }
}

var accelerometer_chart_lines = bind_chart("accelerometer_chart", 3, false);
var gyroscope_chart_lines = bind_chart("gyroscope_chart", 3, false);
var magnetometer_chart_lines = bind_chart("magnetometer_chart", 3, false);
var mpu9250_thermometer_chart_line = bind_chart(
  "mpu9250_thermometer_chart",
  1,
  true
);

var light_chart_line = bind_chart("light_chart", 1, true);

var bme280_thermometer_chart_line = bind_chart(
  "bme280_thermometer_chart",
  1,
  true
);
var hygrometer_chart_line = bind_chart("hygrometer_chart", 1, true);
var barometer_chart_line = bind_chart("barometer_chart", 1, true);
var bme280_altimeter_chart_line = bind_chart("bme280_altimeter_chart", 1, true);

var speedometer_chart_line = bind_chart("speedometer_chart", 1, true);
var gps_altimeter_chart_line = bind_chart("gps_altimeter_chart", 1, true);

var mq2_chart_line = bind_chart("mq2_chart", 1, true);
var mq3_chart_line = bind_chart("mq3_chart", 1, true);
var mq4_chart_line = bind_chart("mq4_chart", 1, true);
var mq5_chart_line = bind_chart("mq5_chart", 1, true);
var mq6_chart_line = bind_chart("mq6_chart", 1, true);
var mq7_chart_line = bind_chart("mq7_chart", 1, true);
var mq8_chart_line = bind_chart("mq8_chart", 1, true);
var mq9_chart_line = bind_chart("mq9_chart", 1, true);
var mq135_chart_line = bind_chart("mq135_chart", 1, true);

if (!!window.EventSource) {
  var source = new EventSource("/Events");
  source.addEventListener(
    "open",
    function () {
      Show_Alert("Connected");
    },
    false
  );
  source.addEventListener(
    "error",
    function (e) {
      if (e.target.readyState != EventSource.OPEN) {
        Show_Alert("Disconnected");
      }
    },
    false
  );
  source.addEventListener(
    "json",
    function (crude_json) {
      const current_time = new Date().getTime();

      var UpTime_Milliseconds = parseInt(crude_json.lastEventId);
      var Parsed_JSON = JSON.parse(crude_json.data);

      var UpTime = (UpTime_Milliseconds / 1000).toFixed(2);

      document.getElementById("UpTime").innerHTML = UpTime;

      var acceleration_x = Parsed_JSON.Acceleration_X;
      var acceleration_y = Parsed_JSON.Acceleration_Y;
      var acceleration_z = Parsed_JSON.Acceleration_Z;

      accelerometer_chart_lines[0].append(current_time, acceleration_x);
      accelerometer_chart_lines[1].append(current_time, acceleration_y);
      accelerometer_chart_lines[2].append(current_time, acceleration_z);

      document.getElementById("Acceleration_X").innerHTML = acceleration_x;
      document.getElementById("Acceleration_Y").innerHTML = acceleration_y;
      document.getElementById("Acceleration_Z").innerHTML = acceleration_z;
      document.getElementById("Resultant_Acceleration").innerHTML =
        Parsed_JSON.Resultant_Acceleration;

      var gyro_x = Parsed_JSON.Gyro_X;
      var gyro_y = Parsed_JSON.Gyro_Y;
      var gyro_z = Parsed_JSON.Gyro_Z;

      gyroscope_chart_lines[0].append(current_time, gyro_x);
      gyroscope_chart_lines[1].append(current_time, gyro_y);
      gyroscope_chart_lines[2].append(current_time, gyro_z);

      document.getElementById("Gyro_X").innerHTML = gyro_x;
      document.getElementById("Gyro_Y").innerHTML = gyro_y;
      document.getElementById("Gyro_Z").innerHTML = gyro_z;

      var magneto_x = Parsed_JSON.Magneto_X;
      var magneto_y = Parsed_JSON.Magneto_Y;
      var magneto_z = Parsed_JSON.Magneto_Z;

      magnetometer_chart_lines[0].append(current_time, magneto_x);
      magnetometer_chart_lines[1].append(current_time, magneto_y);
      magnetometer_chart_lines[2].append(current_time, magneto_z);

      document.getElementById("Magneto_X").innerHTML = magneto_x;
      document.getElementById("Magneto_Y").innerHTML = magneto_y;
      document.getElementById("Magneto_Z").innerHTML = magneto_z;

      var light = Parsed_JSON.Light;
      light_chart_line[0].append(current_time, light);
      document.getElementById("Light").innerHTML = light;

      var temperature_mpu9250 = Parsed_JSON.Temperature_MPU9250;
      mpu9250_thermometer_chart_line[0].append(
        current_time,
        temperature_mpu9250
      );
      document.getElementById("Temperature_MPU9250").innerHTML =
        temperature_mpu9250;

      var temperature_bme280 = Parsed_JSON.Temperature_BME280;
      var humidity = Parsed_JSON.Humidity;
      var pressure = Parsed_JSON.Pressure;
      var altitude_bme280 = Parsed_JSON.Altitude_BME280;

      bme280_thermometer_chart_line[0].append(current_time, temperature_bme280);
      hygrometer_chart_line[0].append(current_time, humidity);
      barometer_chart_line[0].append(current_time, pressure);
      bme280_altimeter_chart_line[0].append(current_time, altitude_bme280);

      document.getElementById("Temperature_BME280").innerHTML =
        temperature_bme280;
      document.getElementById("Humidity").innerHTML = humidity;
      document.getElementById("Pressure").innerHTML = pressure;
      document.getElementById("Altitude_BME280").innerHTML = altitude_bme280;

      var speed = Parsed_JSON.Speed;
      var altitude_gps = Parsed_JSON.Altitude_GPS;

      speedometer_chart_line[0].append(current_time, speed);
      gps_altimeter_chart_line[0].append(current_time, altitude_gps);

      document.getElementById("Latitude").innerHTML = Parsed_JSON.Latitude;
      document.getElementById("Longitude").innerHTML = Parsed_JSON.Longitude;
      document.getElementById("Speed").innerHTML = speed;
      document.getElementById("Course").innerHTML = Parsed_JSON.Course;
      document.getElementById("Altitude_GPS").innerHTML = altitude_gps;
      document.getElementById("Satellites").innerHTML = Parsed_JSON.Satellites;
      document.getElementById("HDOP").innerHTML = Parsed_JSON.HDOP;

      var mq2 = Parsed_JSON.MQ_2;
      var mq3 = Parsed_JSON.MQ_3;
      var mq4 = Parsed_JSON.MQ_4;
      var mq5 = Parsed_JSON.MQ_5;
      var mq6 = Parsed_JSON.MQ_6;
      var mq7 = Parsed_JSON.MQ_7;
      var mq8 = Parsed_JSON.MQ_8;
      var mq9 = Parsed_JSON.MQ_9;
      var mq135 = Parsed_JSON.MQ_135;

      mq2_chart_line[0].append(current_time, mq2);
      mq3_chart_line[0].append(current_time, mq3);
      mq4_chart_line[0].append(current_time, mq4);
      mq5_chart_line[0].append(current_time, mq5);
      mq6_chart_line[0].append(current_time, mq6);
      mq7_chart_line[0].append(current_time, mq7);
      mq8_chart_line[0].append(current_time, mq8);
      mq9_chart_line[0].append(current_time, mq9);
      mq135_chart_line[0].append(current_time, mq135);

      document.getElementById("MQ_2").innerHTML = mq2;
      document.getElementById("MQ_3").innerHTML = mq3;
      document.getElementById("MQ_4").innerHTML = mq4;
      document.getElementById("MQ_5").innerHTML = mq5;
      document.getElementById("MQ_6").innerHTML = mq6;
      document.getElementById("MQ_7").innerHTML = mq7;
      document.getElementById("MQ_8").innerHTML = mq8;
      document.getElementById("MQ_9").innerHTML = mq9;
      document.getElementById("MQ_135").innerHTML = mq135;

      document.getElementById("Slave_UpTime").innerHTML = (
        parseInt(Parsed_JSON.Slave_UpTime) / 1000
      ).toFixed(2);

      if (parseInt(Parsed_JSON.BlackBody_Motion) == 1) {
        document.getElementById("BlackBody_Motion").style.display = "flex";
      } else {
        document.getElementById("BlackBody_Motion").style.display = "none";
      }

      if (parseInt(Parsed_JSON.Flame) == 1) {
        document.getElementById("Flame").style.display = "flex";
      } else {
        document.getElementById("Flame").style.display = "none";
      }
    },
    false
  );
}

Array.prototype.forEach.call(
  document.querySelectorAll(".relay_button"),
  function (relay_button) {
    relay_button.onclick = function () {
      fetch(
        "/Relays?Number=" +
          relay_button.dataset.number +
          "&State=" +
          relay_button.dataset.state,
        {
          method: "GET",
        }
      )
        .then(function (response) {
          return response.text();
        })
        .then(function (realy_state) {
          Show_Alert(realy_state);
        });
    };
  }
);

var Morse_Code_Form = document.getElementById("morse_code_form");
Morse_Code_Form.onsubmit = function (submission) {
  submission.preventDefault();

  var Message_Field = Morse_Code_Form.message;
  var Submit_Button = document.getElementById("morse_code_submit_button");

  Submit_Button.value = "Sending";
  fetch("/Morse_Code?String=" + Message_Field.value, {
    method: "GET",
  })
    .then(function (response) {
      return response.text();
    })
    .then(function (Response_Text) {
      Submit_Button.value = "Send";
      Show_Alert(Response_Text);
    });
};

var Scan_WiFi_Button = document.getElementById("Scan_WiFi");
Scan_WiFi_Button.onclick = function () {
  Scan_WiFi_Button.innerHTML = "Scanning";

  fetch("/Scan_WiFi", {
    method: "GET",
  })
    .then(function (response) {
      return response.text();
    })
    .then(function (WiFi_Scan_Result) {
      if (WiFi_Scan_Result == "") {
        Scan_WiFi_Button.click();
      } else {
        document.getElementById("WiFi_Scan_Result").innerHTML =
          WiFi_Scan_Result;
        Scan_WiFi_Button.innerHTML = "Scan";
        Show_Alert("WiFi Scanned");
      }
    });
};

var Scan_I2C_Button = document.getElementById("Scan_I2C");
Scan_I2C_Button.onclick = function () {
  Scan_I2C_Button.innerHTML = "Scanning";
  fetch("/Scan_I2C", {
    method: "GET",
  })
    .then(function (response) {
      return response.text();
    })
    .then(function (I2C_Scan_Result) {
      document.getElementById("I2C_Scan_Result").innerHTML = I2C_Scan_Result;
      Scan_I2C_Button.innerHTML = "Scan";
      Show_Alert("I2C Bus Scanned");
    });
};

var ReBoot_Button = document.getElementById("reboot_button");
ReBoot_Button.onclick = function () {
  ReBoot_Button.innerHTML = "Requesting";
  fetch("/ReBoot?Confirm=true", {
    method: "GET",
  })
    .then(function (response) {
      return response.text();
    })
    .then(function (Response_Text) {
      ReBoot_Button.innerHTML = "Reboot";
      Show_Alert(Response_Text);
    });
};
