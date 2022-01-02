// HTML web page to handle input fields
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <title>Name Plate Configuration</title>
    <meta name="viewport" content="width=device-width, initial-scale=1" />
    <style>
      body {
        background-color: #ffffff;
        font-family: Georgia, "Times New Roman", Times, serif;
        color: rgb(0, 0, 0);
        color: rgb(0, 0, 0);
      }

      input[type="text"] {
        width: 100px;
        height: 20px;
        border: 1px solid rgba(0, 0, 0, 0.349);
        background: rgb(203, 204, 193);
      }
    </style>
  </head>

  <body>
    <center>
      <form action="/get">
        <h2 align="center">Name Plate Configuration Form</h2>

        <h3>Enter Wi-Fi Details</h3>
        <table>
          <tr>
            <td>SSID:</td>
            <td><input type="text" name="ssid" placeholder="Enter SSID" /></td>
          </tr>

          <tr>
            <td>Password:</td>
            <td>
              <input type="text" name="password" placeholder="Enter Password" />
            </td>
          </tr>
        </table>

        <h3>Enter Time Configurations</h3>
        <table>
          <tr>
            <td>Start Time:</td>
            <td>
              <input type="text" name="start_hour" placeholder="Start Hour" />
            </td>
            <td>
              <input
                type="text"
                name="start_minutes"
                placeholder="Start Minutes"
              />
            </td>
          </tr>

          <tr>
            <td>Stop Time:</td>
            <td>
              <input type="text" name="stop_hour" placeholder="Stop Hour" />
            </td>
            <td>
              <input
                type="text"
                name="stop_minutes"
                placeholder="Stop Minutes"
              />
            </td>
          </tr>
        </table>
        <br />
        <input type="submit" value="Submit" />
      </form>
    </center>
  </body>
</html>

)rawliteral";
