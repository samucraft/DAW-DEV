import cv2
import fcntl
from picamera2 import Picamera2
import time

GESTURE_NOHAND = "0"
GESTURE_CLOSED = "1"
GESTURE_OPENED = "2"

GESTURE_FILE = "/tmp/gesture.txt"

def write_gesture(value):
    with open(GESTURE_FILE, 'w') as f:
        fcntl.flock(f, fcntl.LOCK_EX)  # exclusive lock
        f.write(value + "\n")
        f.flush()
        fcntl.flock(f, fcntl.LOCK_UN)  # unlock

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
        print(f"Contour area: {cv2.contourArea(cnt)}")
        if cv2.contourArea(cnt) > 2500:
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
            if gesture == "No Hand":
                write_gesture(GESTURE_NOHAND)
            elif gesture == "Closed Fist":
                write_gesture(GESTURE_CLOSED)
            else:
                write_gesture(GESTURE_OPENED)

        time.sleep(0.5)  # Adjust delay as needed

except KeyboardInterrupt:
    print("Exiting gesture detection.")
