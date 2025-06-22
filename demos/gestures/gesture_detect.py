import cv2
import numpy as np
from picamera2 import Picamera2
import time

def count_fingers(contour):
    hull = cv2.convexHull(contour, returnPoints=False)
    if len(hull) > 3:
        defects = cv2.convexityDefects(contour, hull)
        if defects is not None:
            count = 0
            for i in range(defects.shape[0]):
                s, e, f, d = defects[i, 0]
                if d > 1000:
                    count += 1
            return count
    return 0

def detect_gesture(frame):
    roi = frame[60:240, 60:240]  # Focus on center
    gray = cv2.cvtColor(roi, cv2.COLOR_BGR2GRAY)
    blur = cv2.GaussianBlur(gray, (35, 35), 0)
    _, thresh = cv2.threshold(blur, 80, 255, cv2.THRESH_BINARY_INV+cv2.THRESH_OTSU)

    contours, _ = cv2.findContours(thresh, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
    if contours:
        cnt = max(contours, key=cv2.contourArea)
        if cv2.contourArea(cnt) > 5000:
            finger_count = count_fingers(cnt)
            if finger_count <= 1:
                return "Closed Fist"
            else:
                return "Open Hand"
    return "No Hand"

# Initialize camera
picam2 = Picamera2()
picam2.configure(picam2.create_preview_configuration(
    main={"format": 'XRGB8888', "size": (320, 240)}
))
picam2.start()
time.sleep(2)  # Allow warm-up

print("Starting gesture detection. Press Ctrl+C to stop.")

try:
    last_gesture = ""
    while True:
        frame = picam2.capture_array()
        gesture = detect_gesture(frame)

        if gesture != last_gesture:
            print(f"Detected gesture: {gesture}")
            last_gesture = gesture

        time.sleep(0.5)  # Adjust delay as needed

except KeyboardInterrupt:
    print("Exiting gesture detection.")
