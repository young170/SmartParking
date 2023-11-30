import cv2
import cvlib as cv
import urllib.request
import numpy as np
from cvlib.object_detection import draw_bbox
import multiprocessing
import paho.mqtt.client as mqtt
import time
import json

def on_message(client, userdata, message, car_count):
    payload = message.payload.decode("utf-8")
    print(f"Received message: {payload}")
    car_count.value = int(payload.split(":")[1].strip())

def on_data_message(client, userdata, message, distance, cam_on_flag, car_in_flag, car_count, mqtt_topic_car):
    payload = message.payload.decode("utf-8")
    print(f"Received data message: {payload}")

    try:
        data = json.loads(payload)
        dist_value = data.get('dist')

        if dist_value is not None:
            distance.value = float(dist_value)
            if distance.value <= 10 and not car_in_flag.value:
                cam_on_flag.value = 10
                car_in_flag.value = True
            elif distance.value > 10 and car_in_flag.value:
                cam_on_flag.value = 10
                car_in_flag.value = False
                

                # Publish the updated car count
                client.publish(mqtt_topic_car, f'Car Count: {car_count.value}')

    except json.JSONDecodeError as e:
        print(f"Error decoding JSON: {e}")

def setup_mqtt(mqtt_broker, on_message_callback, mqtt_topic):
    client = mqtt.Client()
    client.on_message = on_message_callback
    client.connect(mqtt_broker)
    client.subscribe(mqtt_topic)
    client.loop_start()
    return client

def run2(url, mqtt_broker, mqtt_topic_car, car_count, cam_on_flag, published_time, car_in_flag, distance):
    cv2.namedWindow("detection", cv2.WINDOW_AUTOSIZE)

    client = setup_mqtt(mqtt_broker, lambda client, userdata, message: on_message(client, userdata, message, car_count), mqtt_topic_car)

    while True:
        try:
            with urllib.request.urlopen(url) as img_resp:
                imgnp = np.array(bytearray(img_resp.read()), dtype=np.uint8)
                im = cv2.imdecode(imgnp, -1)

            bbox, label, conf = cv.detect_common_objects(im)

            if cam_on_flag.value > 0:
                car_count.value = label.count('car') + label.count('bus') + label.count('truck')
                im = draw_bbox(im, bbox, label, conf)
                cv2.putText(im, f'Car Count: {car_count.value}', (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
                print("car count: ", car_count.value)

            cv2.imshow('detection', im)
            key = cv2.waitKey(1) & 0xFF
            if key == ord('q'):
                break

            if cam_on_flag.value > 0:
                cam_on_flag.value -= 1
                current_time = time.time()
                if current_time - published_time.value >= 1:
                    # Publish the updated car count
                    client.publish(mqtt_topic_car, f'Car Count: {car_count.value}')
                    published_time.value = current_time

        except Exception as e:
            print(f"Error in run2: {e}")

    client.loop_stop()
    cv2.destroyAllWindows()

def run3(mqtt_broker, mqtt_topic_data, distance, cam_on_flag, car_in_flag, car_count, mqtt_topic_car):
    client = setup_mqtt(
        mqtt_broker,
        lambda client, userdata, message: on_data_message(client, userdata, message, distance, cam_on_flag, car_in_flag, car_count, mqtt_topic_car),
        mqtt_topic_data
    )
    while True:
        time.sleep(1) 

if __name__ == '__main__':
    manager = multiprocessing.Manager()
    car_count = manager.Value('i', 0)
    published_time = manager.Value('d', 0.0)
    cam_on_flag = manager.Value('i', 0)
    car_in_flag = manager.Value('b', False)
    distance = manager.Value('f', 0.0)

    # MQTT setup
    mqtt_broker = "sweetdream.iptime.org"
    mqtt_topic_car = "iot/5/car"
    mqtt_topic_data = "iot/5/data"

    url = 'http://192.168.0.171/cam-hi.jpg'

    print("started")

    process2 = multiprocessing.Process(target=run2, args=(url, mqtt_broker, mqtt_topic_car, car_count, cam_on_flag, published_time, car_in_flag, distance))
    process3 = multiprocessing.Process(target=run3, args=(mqtt_broker, mqtt_topic_data, distance, cam_on_flag, car_in_flag, car_count, mqtt_topic_car))

    process2.start()
    process3.start()

    process2.join()
    process3.join()
