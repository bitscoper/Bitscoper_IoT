/* By Abdullah As-Sadeed */

function Show_Alert(Message) {
  if (typeof Message !== "undefined") {
    if (Message !== "") {
      var Alert_Box = document.createElement("div");
      Alert_Box.innerHTML = Message;
      Alert_Box.classList.add("Alert_Box");
      Alert_Box.classList.add("show");
      document.body.append(Alert_Box);

      setTimeout(function () {
        Alert_Box.classList.remove("show");
      }, 3000);

      setTimeout(function () {
        Alert_Box.remove();
      }, 4000);
    } else {
      return false;
    }
  } else {
    return false;
  }
}

document.oncontextmenu = function (Context_Menu) {
  Context_Menu.preventDefault();

  Show_Alert("Context menu is not allowed!");

  return false;
};

document.onkeydown = function (Key_Board) {
  if (
    Key_Board.key == "F12" ||
    (Key_Board.ctrlKey && Key_Board.shiftKey && Key_Board.key == "i") ||
    (Key_Board.ctrlKey && Key_Board.shiftKey && Key_Board.key == "j") ||
    (Key_Board.ctrlKey && Key_Board.key == "u")
  ) {
    Key_Board.preventDefault();

    Show_Alert("Developer tools are not allowed!");

    return false;
  } else if (
    Key_Board.Code == "PrintScreen" ||
    (Key_Board.ctrlKey && Key_Board.key == "p")
  ) {
    Key_Board.preventDefault();

    Show_Alert("Printing is not allowed!");

    return false;
  } else if (Key_Board.ctrlKey && Key_Board.key == "s") {
    Key_Board.preventDefault();

    Show_Alert("Saving is not allowed!");

    return false;
  }
};

function Place_Value(Element_ID, Value) {
  if (typeof Value !== "undefined") {
    document.getElementById(Element_ID).innerHTML = Value;
  } else {
    return false;
  }
}

if (!!window.EventSource) {
  var source = new EventSource("/SSE");

  source.addEventListener(
    "open",
    function () {
      Show_Alert("Connected");
    },
    false
  );

  source.addEventListener(
    "error",
    function (Error) {
      if (Error.target.readyState != EventSource.OPEN) {
        Show_Alert("Disconnected");
      }
    },
    false
  );

  source.addEventListener(
    "JSON",
    function (JSON_String) {
      var ESP32_UpTime = parseInt(JSON_String.lastEventId) / 1000;
      var Parsed_JSON = JSON.parse(JSON_String.data);

      Place_Value(
        "Arduino_Mega_2650_Compilation_Date_and_Time",
        Parsed_JSON["Compilation_Date_and_Time"]
      );

      Place_Value("Arduino_Mega_2560_UpTime", Parsed_JSON["UpTime"]);

      Place_Value("I2C_Devices", Parsed_JSON["I2C_Devices"]);

      Place_Value("ESP32_UpTime", ESP32_UpTime);

      Place_Value(
        "MPU9250_Acceleration_X",
        Parsed_JSON["MPU9250"].Acceleration.X
      );
      Place_Value(
        "MPU9250_Acceleration_Y",
        Parsed_JSON["MPU9250"].Acceleration.Y
      );
      Place_Value(
        "MPU9250_Acceleration_Z",
        Parsed_JSON["MPU9250"].Acceleration.Z
      );
      Place_Value(
        "MPU9250_Acceleration_Resultant",
        Parsed_JSON["MPU9250"].Acceleration.Resultant
      );

      Place_Value("MPU9250_Gyro_X", Parsed_JSON["MPU9250"].Gyro.X);
      Place_Value("MPU9250_Gyro_Y", Parsed_JSON["MPU9250"].Gyro.Y);
      Place_Value("MPU9250_Gyro_Z", Parsed_JSON["MPU9250"].Gyro.Z);

      Place_Value("MPU9250_Magneto_X", Parsed_JSON["MPU9250"].Magneto.X);
      Place_Value("MPU9250_Magneto_Y", Parsed_JSON["MPU9250"].Magneto.Y);
      Place_Value("MPU9250_Magneto_Z", Parsed_JSON["MPU9250"].Magneto.Z);

      Place_Value("MPU9250_Temperature", Parsed_JSON["MPU9250"].Temperature);

      Place_Value("BH1750_Light", Parsed_JSON["BH1750"]);

      Place_Value("BME280_Temperature", Parsed_JSON["BME280"].Temperature);
      Place_Value("BME280_Humidity", Parsed_JSON["BME280"].Humidity);
      Place_Value("BME280_Pressure", Parsed_JSON["BME280"].Pressure);
      Place_Value("BME280_Altitude", Parsed_JSON["BME280"].Altitude);

      Place_Value("DSM501A_PM25_mgm3", Parsed_JSON["DSM501A"].PM25_mgm3);
      Place_Value(
        "DSM501A_PM25_pcs283ml",
        Parsed_JSON["DSM501A"].PM25_pcs283ml
      );

      Place_Value("DSM501A_PM1_mgm3", Parsed_JSON["DSM501A"].PM1_mgm3);
      Place_Value("DSM501A_PM1_pcs283ml", Parsed_JSON["DSM501A"].PM1_pcs283ml);

      Place_Value("MQ2", Parsed_JSON["MQ2"]);
      Place_Value("MQ3", Parsed_JSON["MQ3"]);
      Place_Value("MQ4", Parsed_JSON["MQ4"]);
      Place_Value("MQ5", Parsed_JSON["MQ5"]);
      Place_Value("MQ6", Parsed_JSON["MQ6"]);
      Place_Value("MQ7", Parsed_JSON["MQ7"]);
      Place_Value("MQ8", Parsed_JSON["MQ8"]);
      Place_Value("MQ9", Parsed_JSON["MQ9"]);
      Place_Value("MQ135", Parsed_JSON["MQ135"]);

      Place_Value("IR_Radiation", Parsed_JSON["IR_Radiation"]);

      Place_Value("HC_SR501", Parsed_JSON["Motion"].HC_SR501);
      Place_Value("RCWL0516", Parsed_JSON["Motion"].RCWL0516);

      Place_Value("RDM6300_Card", Parsed_JSON["RDM6300"]);

      Place_Value("RC522_PICC_Type", Parsed_JSON["RC522"].PICC_Type);
      Place_Value(
        "RC522_MIFARE_Classic_Validity",
        Parsed_JSON["RC522"].MIFARE_Classic_Validity
      );
      Place_Value("RC522_UID", Parsed_JSON["RC522"].UID);

      Place_Value("NEO7M_Satellites", Parsed_JSON["NEO7M"].Satellites);
      Place_Value("NEO7M_Latitude", Parsed_JSON["NEO7M"].Latitude);
      Place_Value("NEO7M_Longitude", Parsed_JSON["NEO7M"].Longitude);
      Place_Value("NEO7M_Speed", Parsed_JSON["NEO7M"].Speed);
      Place_Value("NEO7M_Course", Parsed_JSON["NEO7M"].Course);
      Place_Value("NEO7M_Altitude", Parsed_JSON["NEO7M"].Altitude);
      Place_Value("NEO7M_HDOP", Parsed_JSON["NEO7M"].HDOP);

      Place_Value("DS3231_Time", Parsed_JSON["DS3231"].Time);

      Place_Value("DS3231_Alarm_1_Time", Parsed_JSON["DS3231"].Alarm_1.Time);
      Place_Value("DS3231_Alarm_1_Mode", Parsed_JSON["DS3231"].Alarm_1.Mode);
      Place_Value(
        "DS3231_Alarm_1_Is_Fired",
        Parsed_JSON["DS3231"].Alarm_1.Is_Fired
      );

      Place_Value("DS3231_Alarm_2_Time", Parsed_JSON["DS3231"].Alarm_2.Time);
      Place_Value("DS3231_Alarm_2_Mode", Parsed_JSON["DS3231"].Alarm_2.Mode);
      Place_Value(
        "DS3231_Alarm_2_Is_Fired",
        Parsed_JSON["DS3231"].Alarm_2.Is_Fired
      );

      Place_Value("DS3231_Temperature", Parsed_JSON["DS3231"].Temperature);
    },
    false
  );
}

Array.prototype.forEach.call(
  document.querySelectorAll(".ReSet_Button"),
  function (ReSet_Button) {
    ReSet_Button.onclick = function () {
      ReSet_Button.innerHTML = "Requesting";

      fetch(
        '/Requests?JSON={"ReSet_' + ReSet_Button.dataset.device + '": true}',
        {
          method: "GET",
        }
      )
        .then(function (response) {
          return response.text();
        })
        .then(function (Response_Text) {
          ReSet_Button.innerHTML = "Reboot";
          Show_Alert(Response_Text);
        });
    };
  }
);

var I2C_Scan_Button = document.getElementById("I2C_Scan_Button");
I2C_Scan_Button.onclick = function () {
  I2C_Scan_Button.innerHTML = "Scanning";

  fetch('/Requests?JSON={"Scan_I2C": ""}', {
    method: "GET",
  })
    .then(function (Response) {
      return Response.text();
    })
    .then(function (Response_Text) {
      Show_Alert(Response_Text);

      I2C_Scan_Button.innerHTML = "Scan";
    });
};

Array.prototype.forEach.call(
  document.querySelectorAll(".SG90_State_Button"),
  function (SG90_State_Button) {
    SG90_State_Button.onclick = function () {
      fetch(
        '/Requests?JSON={"' + SG90_State_Button.dataset.state + '_SG90": true}',
        {
          method: "GET",
        }
      )
        .then(function (Response) {
          return Response.text();
        })
        .then(function (Response_Text) {
          Show_Alert(Response_Text);
        });
    };
  }
);

var SG90_Position_Slider = document.getElementById("SG90_Position_Slider");
SG90_Position_Slider.oninput = function () {
  fetch(
    '/Requests?JSON={"SG90_Position": ' + SG90_Position_Slider.value + "}",
    {
      method: "GET",
    }
  )
    .then(function (Response) {
      return Response.text();
    })
    .then(function (Response_Text) {
      Show_Alert(Response_Text);
    });
};

Array.prototype.forEach.call(
  document.querySelectorAll(".ULN2003_Step_Button"),
  function (ULN2003_Step_Button) {
    ULN2003_Step_Button.onclick = function () {
      fetch(
        '/Requests?JSON={"ULN2003_Steps": ' +
          ULN2003_Step_Button.dataset.direction +
          document.getElementById("ULN2003_Steps_Input").value +
          "}",
        {
          method: "GET",
        }
      )
        .then(function (Response) {
          return Response.text();
        })
        .then(function (Response_Text) {
          Show_Alert(Response_Text);
        });
    };
  }
);

Array.prototype.forEach.call(
  document.querySelectorAll(".Relay_Button"),
  function (Relay_Button) {
    Relay_Button.onclick = function () {
      fetch(
        '/Requests?JSON={"Relay_' +
          Relay_Button.dataset.number +
          '": ' +
          Relay_Button.dataset.state +
          "}",
        {
          method: "GET",
        }
      )
        .then(function (Response) {
          return Response.text();
        })
        .then(function (Response_Text) {
          Show_Alert(Response_Text);
        });
    };
  }
);

var WiFi_Scan_Button = document.getElementById("WiFi_Scan_Button");
WiFi_Scan_Button.onclick = function () {
  WiFi_Scan_Button.innerHTML = "Scanning";

  fetch('/Requests?JSON={"Scan_WiFi": ""}', {
    method: "GET",
  })
    .then(function (Response) {
      return Response.text();
    })
    .then(function (WiFi_Scan_Result) {
      if (WiFi_Scan_Result == "") {
        WiFi_Scan_Button.click();
      } else {
        document.getElementById("WiFi_Scan_Result").innerHTML =
          WiFi_Scan_Result;

        WiFi_Scan_Button.innerHTML = "Scan";
      }
    });
};
