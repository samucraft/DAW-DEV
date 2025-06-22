import cv2
import numpy as np
import signal
import sys
import time

# Flag to allow clean shutdown
running = True

def signal_handler(sig, frame):
    global running
    running = False
    print("\nExiting...")
    sys.exit(0)

# Handle Ctrl+C
signal.signal(signal.SIGINT, signal_handler)

# Start capturing from the CSI or USB camera
cap = cv2.VideoCapture(0)
if not cap.isOpened():
    print("Error: Could not open video device.")
    sys.exit(1)

print("Starting hand gesture detection (press Ctrl+C to stop)...")

while running:
    ret, frame = cap.read()
    if not ret:
        print("Error: Failed to grab frame.")
        break

    frame = cv2.flip(frame, 1)
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

    # Skin color range for HSV
    lower = np.array([0, 30, 60], dtype=np.uint8)
    upper = np.array([20, 150, 255], dtype=np.uint8)
    mask = cv2.inRange(hsv, lower, upper)

    # Clean the mask
    mask = cv2.GaussianBlur(mask, (5, 5), 0)
    contours, _ = cv2.findContours(mask, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)

    gesture = "No hand detected"

    if contours:
        max_contour = max(contours, key=cv2.contourArea)
        hull = cv2.convexHull(max_contour, returnPoints=False)
        if hull is not None and len(hull) > 3:
            defects = cv2.convexityDefects(max_contour, hull)
            if defects is not None:
                finger_count = 0
                for i in range(defects.shape[0]):
                    s, e, f, d = defects[i, 0]
                    start = tuple(max_contour[s][0])
                    end = tuple(max_contour[e][0])
                    far = tuple(max_contour[f][0])
                    a = np.linalg.norm(np.array(start) - np.array(end))
                    b = np.linalg.norm(np.array(far) - np.array(start))
                    c = np.linalg.norm(np.array(far) - np.array(end))
                    angle = np.arccos((b**2 + c**2 - a**2) / (2 * b * c)) * 57

                    if angle <= 90:
                        finger_count += 1

                if finger_count >= 4:
                    gesture = "Open Hand"
                else:
                    gesture = "Closed Fist"

    print(f"Detected Gesture: {gesture}")
    time.sleep(0.5)  # Reduce CPU load and improve readability

cap.release()