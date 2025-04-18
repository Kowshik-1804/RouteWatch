from flask import Flask, request, jsonify
from flask_cors import CORS
from pymongo import MongoClient
from datetime import datetime

# Initialize Flask app
app = Flask(__name__)
CORS(app)

# Connect to MongoDB
try:
    client = MongoClient("mongodb+srv://kowshikshrinivas:kowshikshrinivas@cluster0.evter7m.mongodb.net/?retryWrites=true&w=majority&appName=Cluster0")
    db = client['geoData']
    collection = db['locations']
    print("✅ Connected to MongoDB")
except Exception as e:
    print("❌ MongoDB connection failed:", str(e))

@app.route('/addLocation', methods=['POST'])
def add_location():
    try:
        data = request.get_json()
        print("📦 Received data:", data)

        if not data:
            return jsonify({"status": "fail", "message": "No data received"}), 400

        location_data = {
            "locality": data.get("locality"),
            "city": data.get("city"),
            "state": data.get("state"),
            "timestamp": data.get("timestamp", datetime.now().isoformat())
        }

        print("📝 Inserting into MongoDB:", location_data)

        collection.insert_one(location_data)

        return jsonify({"status": "success", "message": "Location saved"}), 200

    except Exception as e:
        print("💥 Error while inserting data:", str(e))
        return jsonify({"status": "fail", "message": str(e)}), 500

# Run server
if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
