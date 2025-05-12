import cv2
import numpy as np
import serial
import time

# Open serial port
ser = serial.Serial('COM5', 9600)  # Change 'COM17' to your Arduino's serial port

# Load YOLOv3 model
net = cv2.dnn.readNetFromDarknet("yolov3.cfg", "yolov3.weights")
classes = []
with open("coco.names", "r") as f:
    classes = [line.strip() for line in f.readlines()]

layer_names = net.getLayerNames()
output_layers = [layer_names[i - 1] for i in net.getUnconnectedOutLayers()]

# Initialize camera
cap = cv2.VideoCapture(0)

lastDetectionTime = time.time()
lastDetectedObject = None

while True:
    _, frame = cap.read()
    height, width, channels = frame.shape

    # Detecting objects
    blob = cv2.dnn.blobFromImage(frame, 0.00392, (416, 416), (0, 0, 0), True, crop=False)
    net.setInput(blob)
    outs = net.forward(output_layers)

    class_ids = []
    confidences = []
    boxes = []
    detectedObject = None  # Variable to store the current detected object

    for out in outs:
        for detection in out:
            scores = detection[5:]
            class_id = np.argmax(scores)
            confidence = scores[class_id]
            if confidence > 0.5:
                # Object detected
                center_x = int(detection[0] * width)
                center_y = int(detection[1] * height)
                w = int(detection[2] * width)
                h = int(detection[3] * height)

                # Rectangle coordinates
                x = int(center_x - w / 2)
                y = int(center_y - h / 2)

                boxes.append([x, y, w, h])
                confidences.append(float(confidence))
                class_ids.append(class_id)

                detectedObject = str(classes[class_id])  # Store the detected object label

    indexes = cv2.dnn.NMSBoxes(boxes, confidences, 0.5, 0.4)

    if len(indexes) > 0 and detectedObject:
        currentTime = time.time()
        if currentTime - lastDetectionTime >= 60 or detectedObject != lastDetectedObject:
            # Send command based on detected object

            if detectedObject in ["dog", "cat", "cow"]:
                ser.write(b'a')  # Send 'a' for activate_pump
            elif detectedObject == "person":
                ser.write(b'o')  # Send 'o' for open_door
            
            lastDetectionTime = currentTime
            lastDetectedObject = detectedObject
            
            print(f"Command sent: {detectedObject}")
        else:
            print(f"Too soon or same object: {detectedObject}")

    cv2.imshow("Image", frame)
    key = cv2.waitKey(1)
    if key == 27:  # Exit on pressing ESC key
        break

cap.release()
cv2.destroyAllWindows()
