var myAPIKey = '81d7cd5bbf216ebdbff899b3019c69c7';
// Import the Clay package
var Clay = require('pebble-clay');
// Load our Clay configuration file
var clayConfig = require('./config');
// Initialize Clay
var clay = new Clay(clayConfig);


var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function getStock(ticker) {
    var url ="http://download.finance.yahoo.com/d/quotes.csv?s=" + ticker + "&f=l1&e=.csv";
    xhrRequest(url, 'GET', 
    function(responseText) {
        var value = responseText;
        var dictionary = {
            "STOCK_VALUE": value
        };
        // Send to Pebble
        Pebble.sendAppMessage(dictionary,
          function(e) {
            console.log("Stock info sent to Pebble successfully!");
          },
          function(e) {
            console.log("Error sending stock info to Pebble!");
          }
      );
    });
}

function locationSuccess(pos) {
  // Construct URL
  var url = "http://api.openweathermap.org/data/2.5/weather?lat=" +
      pos.coords.latitude + "&lon=" + pos.coords.longitude + '&appid=' + myAPIKey;
  console.log("URL is " + url);
  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);
      console.log("Raw JSON " + JSON.stringify(json));
      // Temperature in Kelvin requires adjustment
      var temperature;
      if (clay.getItemByMessageKey('WEATHER_DEG') == 'C') {
          temperature = Math.round(json.main.temp - 273.15);
      } else {
          temperature = Math.round((json.main.temp - 273.15) * 1.800 + 32.0);
      }    
      //var temperature = 52;
      console.log("Temperature is " + temperature);

      // Conditions
      var conditions = json.weather[0].main;
      //var conditions = "Rain";
      console.log("Conditions are " + conditions);
      
      // Assemble dictionary using our keys
      var dictionary = {
        "WEATHER_TEMP": temperature,
        "WEATHER_COND": conditions
      };

      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log("Weather info sent to Pebble successfully!");
        },
        function(e) {
          console.log("Error sending weather info to Pebble!");
        }
      );
    }      
  );
}

function locationError(err) {
  console.log("Error requesting location!");
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log("PebbleKit JS ready!");
    // Get the initial weather
    getWeather();
    getStock(clay.getItemByMessageKey('STOCK_TICKER'));
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
      if(e.payload.UPDATE_WEATHER) {
          getWeather();
      }
      if(e.payload.UPDATE_STOCK) {
          getStock(clay.getItemByMessageKey('STOCK_TICKER'));
      }
    console.log("AppMessage received!");
    
  }                     
);
