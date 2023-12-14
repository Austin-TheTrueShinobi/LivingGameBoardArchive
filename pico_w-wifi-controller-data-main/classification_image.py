import time
from paz.applications import HaarCascadeFrontalFace, MiniXceptionFER
import paz.processors as pr
from paz.backend import image
import argparse
from paz.backend.image import load_image, show_image
from paz.backend.camera import Camera
import redis

# Redis connection setup
redis_host = 'localhost'  # Redis server address
redis_port = 6379          # Redis server port
redis_db = 0               # Redis database index

# Connect to Redis
redis_client = redis.StrictRedis(host=redis_host, port=redis_port, db=redis_db)

class EmotionDetector(pr.Processor):
    def __init__(self):
        super(EmotionDetector, self).__init__()
        self.detect = HaarCascadeFrontalFace(draw=False)
        self.crop = pr.CropBoxes2D()
        self.classify = MiniXceptionFER()
        print(self.classify)
        self.draw = pr.DrawBoxes2D(self.classify.class_names)

    def call(self, image):
        boxes2D = self.detect(image)['boxes2D']
        cropped_images = self.crop(image, boxes2D)
        for cropped_image, box2D in zip(cropped_images, boxes2D):
            box2D.class_name = self.classify(cropped_image)['class_name']
            print(self.classify(cropped_image)['class_name'])
            redis_client.hset("emotions", "averageMood", self.classify(cropped_image)['class_name'])
        return self.draw(image, boxes2D)

while True:
    detect = EmotionDetector()
    parser = argparse.ArgumentParser(description='Real-time face classifier')
    parser.add_argument('-c', '--camera_id', type=int, default=0,
                        help='Camera device ID')
    parser.add_argument('-o', '--offset', type=float, default=0.1,
                        help='Scaled offset to be added to bounding boxes')
    args = parser.parse_args()

    #pipeline
    camera = Camera(args.camera_id)

    # you can now apply it to an image (numpy array)
    predictions = detect(camera.take_photo())
    time.sleep(3)