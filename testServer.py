from flask import Flask, request, jsonify

app = Flask(__name__)
bus_log = {}

@app.route("/update_bus", methods=["POST"])
def update_bus():
    data = request.get_json()
    bus_number = data["bus_number"]
    route = data["route"]
    
    bus_log[bus_number] = route  # Store in memory (for now)
    
    return jsonify({"message": "Bus info stored", "bus_number": bus_number, "route": route})

@app.route("/get_bus/<bus_number>", methods=["GET"])
def get_bus(bus_number):
    route = bus_log.get(bus_number, "Not Found")
    return jsonify({"bus_number": bus_number, "route": route})

if __name__ == "__main__":
    app.run(debug=True)
