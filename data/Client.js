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
    }
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
  if (typeof Value !== "undefined" && Value !== "") {
    if (
      Element_ID == "Arduino_Mega_2560_UpTime" ||
      Element_ID == "ESP32_CAM_UpTime"
    ) {
      document.getElementById(Element_ID).innerHTML =
        parseFloat(Value / 1000).toFixed(2) + " s";
    } else if (Element_ID == "DS3231_Time") {
      var Time = new Date(Value * 1000);

      document.getElementById(Element_ID).innerHTML = Time.toLocaleString();
    } else if (
      Element_ID == "DS3231_Alarm_1_Time" ||
      Element_ID == "DS3231_Alarm_2_Time"
    ) {
      var Time = new Date(Value * 1000);

      document.getElementById(Element_ID).innerHTML = Time.toLocaleString();
    } else {
      document.getElementById(Element_ID).innerHTML = Value;
    }
  }
}

if (!!window.EventSource) {
  function Connect_SSE() {
    var source = new EventSource("/Server_Sent_Events");

    source.addEventListener(
      "open",
      function () {
        Show_Alert("SSE has been connected.");
      },
      false
    );

    source.addEventListener(
      "error",
      function (Error) {
        if (Error.target.readyState != EventSource.OPEN) {
          Show_Alert("SSE has been disconnected! Reconnecting ...");
          setTimeout(Connect_SSE, 1000);
        }
      },
      false
    );

    source.addEventListener(
      "SSE",
      function (JSON_String) {
        var ESP32_CAM_UpTime = parseInt(JSON_String.lastEventId);
        var Parsed_JSON = JSON.parse(JSON_String.data);

        Place_Value(
          "Arduino_Mega_2560_UpTime",
          Parsed_JSON["Arduino_Mega_2560_UpTime"]
        );

        Place_Value("I2C_Devices", Parsed_JSON["I2C_Devices"]);

        Place_Value("ESP32_CAM_UpTime", ESP32_CAM_UpTime);

        if (Parsed_JSON["MPU9250"]) {
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

          Place_Value(
            "MPU9250_Temperature",
            Parsed_JSON["MPU9250"].Temperature
          );
        }

        Place_Value("BH1750_Light", Parsed_JSON["BH1750"]);

        if (Parsed_JSON["BME280"]) {
          Place_Value("BME280_Temperature", Parsed_JSON["BME280"].Temperature);
          Place_Value("BME280_Humidity", Parsed_JSON["BME280"].Humidity);
          Place_Value("BME280_Pressure", Parsed_JSON["BME280"].Pressure);
          Place_Value("BME280_Altitude", Parsed_JSON["BME280"].Altitude);
        }

        if (Parsed_JSON["DSM501A"]) {
          Place_Value("DSM501A_PM25_mgm3", Parsed_JSON["DSM501A"].PM25_mgm3);
          Place_Value(
            "DSM501A_PM25_pcs283ml",
            Parsed_JSON["DSM501A"].PM25_pcs283ml
          );

          Place_Value("DSM501A_PM1_mgm3", Parsed_JSON["DSM501A"].PM1_mgm3);
          Place_Value(
            "DSM501A_PM1_pcs283ml",
            Parsed_JSON["DSM501A"].PM1_pcs283ml
          );
        }

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

        Place_Value("HC_SR501", Parsed_JSON["HC_SR501"]);

        Place_Value("RCWL0516", Parsed_JSON["RCWL0516"]);

        Place_Value("RDM6300_Reading", Parsed_JSON["RDM6300"]);

        Place_Value("RC522_UID", Parsed_JSON["RC522_UID"]);

        if (Parsed_JSON["NEO7M"]) {
          Place_Value("NEO7M_Satellites", Parsed_JSON["NEO7M"].Satellites);
          Place_Value("NEO7M_Latitude", Parsed_JSON["NEO7M"].Latitude);
          Place_Value("NEO7M_Longitude", Parsed_JSON["NEO7M"].Longitude);
          Place_Value("NEO7M_Speed", Parsed_JSON["NEO7M"].Speed);
          Place_Value("NEO7M_Course", Parsed_JSON["NEO7M"].Course);
          Place_Value("NEO7M_Altitude", Parsed_JSON["NEO7M"].Altitude);
          Place_Value("NEO7M_HDOP", Parsed_JSON["NEO7M"].HDOP);
        }

        if (Parsed_JSON["DS3231"]) {
          Place_Value("DS3231_Time", Parsed_JSON["DS3231"].Time);

          Place_Value(
            "DS3231_Alarm_1_Time",
            Parsed_JSON["DS3231"].Alarm_1.Time
          );
          Place_Value(
            "DS3231_Alarm_1_Mode",
            Parsed_JSON["DS3231"].Alarm_1.Mode
          );
          Place_Value(
            "DS3231_Alarm_1_Is_Fired",
            Parsed_JSON["DS3231"].Alarm_1.Is_Fired
          );

          Place_Value(
            "DS3231_Alarm_2_Time",
            Parsed_JSON["DS3231"].Alarm_2.Time
          );
          Place_Value(
            "DS3231_Alarm_2_Mode",
            Parsed_JSON["DS3231"].Alarm_2.Mode
          );
          Place_Value(
            "DS3231_Alarm_2_Is_Fired",
            Parsed_JSON["DS3231"].Alarm_2.Is_Fired
          );

          Place_Value("DS3231_Temperature", Parsed_JSON["DS3231"].Temperature);
        }
      },
      false
    );
  }

  Connect_SSE();
} else {
  Show_Alert("SSE is not supported by this web browser!");
  console.error("SSE is not supported by this web browser!");
}

document.getElementById("Set_Active_Status_Button").onclick = function () {
  fetch(
    "/Requests?JSON=" +
      JSON.stringify({
        Active_Status: {
          I2C_Devices: document.getElementById("I2C_Devices_Active_Status")
            .checked,
          MPU9250: document.getElementById("MPU9250_Active_Status").checked,
          BH1750: document.getElementById("BH1750_Active_Status").checked,
          BME280: document.getElementById("BME280_Active_Status").checked,
          DSM501A: document.getElementById("DSM501A_Active_Status").checked,
          MQs: document.getElementById("MQs_Active_Status").checked,
          IR: document.getElementById("IR_Active_Status").checked,
          HC_SR501: document.getElementById("HC_SR501_Active_Status").checked,
          RCWL0516: document.getElementById("RCWL0516_Active_Status").checked,
          RDM6300: document.getElementById("RDM6300_Active_Status").checked,
          RC522: document.getElementById("RC522_Active_Status").checked,
          NEO7M: document.getElementById("NEO7M_Active_Status").checked,
          DS3231: document.getElementById("DS3231_Active_Status").checked,
        },
      }),
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
  document.querySelectorAll(".ReBoot_Button"),
  function (ReBoot_Button) {
    ReBoot_Button.onclick = function () {
      fetch(
        '/Requests?JSON={"ReBoot_' + ReBoot_Button.dataset.device + '": true}',
        {
          method: "GET",
        }
      )
        .then(function (response) {
          return response.text();
        })
        .then(function (Response_Text) {
          Show_Alert(Response_Text);
        });
    };
  }
);

Array.prototype.forEach.call(
  document.querySelectorAll(".SG90_State_Button"),
  function (SG90_State_Button) {
    SG90_State_Button.onclick = function () {
      fetch(
        "/Requests?JSON=" +
          JSON.stringify({
            SG90_State: parseInt(SG90_State_Button.dataset.state),
          }),
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
    "/Requests?JSON=" +
      JSON.stringify({
        SG90_Position: parseInt(SG90_Position_Slider.value),
      }),
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
        "/Requests?JSON=" +
          JSON.stringify({
            ULN2003_Steps: parseInt(
              ULN2003_Step_Button.dataset.direction +
                document.getElementById("ULN2003_Steps_InPut").value
            ),
          }),
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
  document.querySelectorAll(".Buzzer_Button"),
  function (Buzzer_Button) {
    Buzzer_Button.onclick = function () {
      fetch(
        "/Requests?JSON=" +
          JSON.stringify({
            Buzzer: parseInt(Buzzer_Button.dataset.state),
          }),
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
        "/Requests?JSON=" +
          JSON.stringify({
            Relay: {
              Number: parseInt(Relay_Button.dataset.number),
              State: parseInt(Relay_Button.dataset.state),
            },
          }),
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

var DS3231_Time_InPut = document.getElementById("DS3231_Time_InPut");
DS3231_Time_InPut.onchange = function () {
  var Time = new Date(DS3231_Time_InPut.value);

  fetch(
    "/Requests?JSON=" +
      JSON.stringify({
        Set_DS3231_Time: Math.floor(Time.getTime() / 1000),
      }),
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

document.getElementById("DS3231_Alarm_1_Time_InPut").onclick = function () {
  Set_DS3231_Alarm(1);
};
document.getElementById("DS3231_Alarm_1_Mode_InPut").onchange = function () {
  Set_DS3231_Alarm(1);
};

document.getElementById("DS3231_Alarm_2_Time_InPut").onclick = function () {
  Set_DS3231_Alarm(2);
};
document.getElementById("DS3231_Alarm_2_Mode_InPut").onchange = function () {
  Set_DS3231_Alarm(2);
};

function Set_DS3231_Alarm(Number) {
  var Time = new Date(
    document.getElementById("DS3231_Alarm_" + Number + "_Time_InPut").value
  );

  fetch(
    "/Requests?JSON=" +
      JSON.stringify({
        Set_DS3231_Alarm: {
          Number: Number,
          Time: Math.floor(Time.getTime() / 1000),
          Mode: document.getElementById(
            "DS3231_Alarm_" + Number + "_Mode_InPut"
          ).value,
        },
      }),
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
}

Array.prototype.forEach.call(
  document.querySelectorAll(".DS3231_Alarm_Clear_Button"),
  function (DS3231_Alarm_Clear_Button) {
    DS3231_Alarm_Clear_Button.onclick = function () {
      fetch(
        "/Requests?JSON=" +
          JSON.stringify({
            Clear_DS3231_Alarm: parseInt(
              DS3231_Alarm_Clear_Button.dataset.number
            ),
          }),
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
  document.querySelectorAll(".Flash_LED_Button"),
  function (Flash_LED_Button) {
    Flash_LED_Button.onclick = function () {
      fetch(
        "/Requests?JSON=" +
          JSON.stringify({
            Flash_LED: parseInt(Flash_LED_Button.dataset.state),
          }),
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
  fetch(
    "/Requests?JSON=" +
      JSON.stringify({
        Scan_WiFi: "",
      }),
    {
      method: "GET",
    }
  )
    .then(function (Response) {
      return Response.text();
    })
    .then(function (WiFi_Scan_Result) {
      if (WiFi_Scan_Result == "") {
        WiFi_Scan_Button.click();
      } else {
        document.getElementById("WiFi_Scan_Result").innerHTML =
          WiFi_Scan_Result;
      }
    });
};

document.getElementById("SIM900A_AT_Command_Button").onclick = function () {
  fetch(
    "/Requests?JSON=" +
      JSON.stringify({
        SIM900A_AT: document.getElementById("SIM900A_AT_InPut").value,
      }),
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
