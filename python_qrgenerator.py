import qrcode
import json

# Define bus data
buses = [
    {"bus_number": "101", "route": "A1"},
    {"bus_number": "202", "route": "B2"}
]

# Generate QR codes for each bus
for bus in buses:
    qr_data = json.dumps(bus)  # Convert dictionary to JSON string
    qr = qrcode.make(qr_data)  # Generate QR code
    file_name = f"bus_{bus['bus_number']}.png"
    qr.save(file_name)
    print(f"QR Code saved: {file_name}")
