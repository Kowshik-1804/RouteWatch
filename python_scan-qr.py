import cv2
import pyzbar.pyzbar as pyzbar
import json
def scan_qr_from_image(image_path):
    image = cv2.imread(image_path)
    decoded_objects = pyzbar.decode(image)

    for obj in decoded_objects:
        qr_data = obj.data.decode("utf-8")  # Extract QR data
        bus_info = json.loads(qr_data)  # Convert JSON to dict
        print(f"Bus Number: {bus_info['bus_number']}, Route: {bus_info['route']}")
        return bus_info

# Test with an image
scan_qr_from_image("bus_101.png")
