import fcntl

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

gesture_states = [GESTURE_NOHAND, GESTURE_CLOSED, GESTURE_OPENED]

i = 0
while True:
    gesture = ""
    key = input("Insert a gesture and hit Enter: ")
    if key == "0":
        gesture = gesture_states[0]
    elif key == "1":
        gesture = gesture_states[1]
    elif key == "2":
        gesture = gesture_states[2]
    elif key == "q":
        break
    else:
        continue

    write_gesture(gesture)
    print(f"Wrote: {gesture}")
