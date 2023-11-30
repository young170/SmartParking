import cv2
import cvlib as cv
import urllib.request
import numpy as np
from cvlib.object_detection import draw_bbox
import concurrent.futures
import paho.mqtt.client as mqtt
import time

url = 'http://192.168.0.171/cam-hi.jpg'
car_count = 0
published_time = 0

def run1():
    cv2.namedWindow("live transmission", cv2.WINDOW_AUTOSIZE)
    while True:
        img_resp = urllib.request.urlopen(url)
        imgnp = np.array(bytearray(img_resp.read()), dtype=np.uint8)
        im = cv2.imdecode(imgnp, -1)

        cv2.imshow('live transmission', im)
        key = cv2.waitKey(5)
        if key == ord('q'):
            break

    cv2.destroyAllWindows()

def run2():
    global car_count, published_time
    cv2.namedWindow("detection", cv2.WINDOW_AUTOSIZE)

    # MQTT setup
    mqtt_broker = "sweetdream.iptime.org"
    mqtt_topic = "iot/21900764/car"
    client = mqtt.Client()

    while True:
        img_resp = urllib.request.urlopen(url)
        imgnp = np.array(bytearray(img_resp.read()), dtype=np.uint8)
        im = cv2.imdecode(imgnp, -1)

        bbox, label, conf = cv.detect_common_objects(im)
        
        car_count.value = label.count('car') + label.count('bus') + label.count('truck')

        im = draw_bbox(im, bbox, label, conf)
        cv2.putText(im, f'Car Count: {car_count}', (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
        
        cv2.imshow('detection', im)
        key = cv2.waitKey(5)
        if key == ord('q'):
            break

        # Check if 1 second has passed since the last publish
        current_time = time.time()
        if current_time - published_time >= 1:
            # Publish car_count to MQTT topic
            client.connect(mqtt_broker) 
            client.publish(mqtt_topic, f'Car Count: {car_count}')
            published_time = current_time

    cv2.destroyAllWindows()

if __name__ == '__main__':
    print("started")
    with concurrent.futures.ProcessPoolExecutor() as executor:
        # f1 = executor.submit(run1)
        f2 = executor.submit(run2)

