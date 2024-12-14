from flask import Flask, jsonify
import time
import requests
import threading

app = Flask(__name__)

# API link for Springfield, MO
weather_url = "https://api.weather.gov/gridpoints/SGF/44,70/forecast"

# Global data storage
weather_data = {"temp": "N/A", "condition": "N/A", "wind_speed": "N/A", "wind_direction": "N/A"}
last_weather_update = 0

def update_weather_data():
    global weather_data, last_weather_update
    try:
        response = requests.get(weather_url).json()
        period = response['properties']['periods'][0]
        weather_data = {
            "temp": period['temperature'],
            "condition": period['shortForecast'],
            "wind_speed": period.get('windSpeed', 'N/A'),
            "wind_direction": period.get('windDirection', 'N/A')
        }
        last_weather_update = time.time()
    except Exception as e:
        print(f"Weather update error: {e}")
    # Schedule the next weather update for 5 minutes later
    threading.Timer(300, update_weather_data).start()

@app.route('/current_weather', methods=['GET'])
def get_weather():
    current_time = time.strftime('%I:%M %p')
    current_date = time.strftime('%m/%d/%Y')
    # Return the current weather and time as JSON
    return jsonify({
        "temp": weather_data["temp"],
        "condition": weather_data["condition"],
        "wind_speed": weather_data["wind_speed"],
        "wind_direction": weather_data["wind_direction"],
        "time": current_time,
        "date": current_date
    })

if __name__ == '__main__':
    update_weather_data()  # Start initial weather update
    app.run(host='0.0.0.0', port=5000)
