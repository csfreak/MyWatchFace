module.exports = [
  {
    "type": "heading",
    "defaultValue": "CSfreak Watchface Configuration"
  },
  {
    "type": "text",
    "defaultValue": "A few settings for you"
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Weather"
      },
      {
        "type": "select",
        "messageKey": "WEATHER_DEG",
        "label": "Temperature",
        "defaultValue": "F",
        "options": [
            { 
              "label": "Farenheit", 
              "value": "F" 
            },
            { 
              "label": "Celcius", 
              "value": "C" 
            }
        ]
      },
      {
        "type": "toggle",
        "messageKey": "LOC_TYPE",
        "defaultValue": true,
        "label": "Automatic Location"
      },
      {
          "type": "input",
          "id": "",
          "messageKey": "LOC_ZIP",
          "defaultValue": "",
          "label": "ZIP Code",
          "attributes": {
              "limit": 5,
              "type": "number"
          }
      }  
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Stock"
      },
      {
        "type": "INPUT",
        "messageKey": "STOCK_TICKER",
        "label": "Ticker Symbol",
        "defaultValue": "VSAT"
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];

