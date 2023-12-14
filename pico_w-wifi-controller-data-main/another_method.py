import time
from paz.applications import HaarCascadeFrontalFace, MiniXceptionFER
import paz.processors as pr
from paz.backend import image
import argparse
from paz.backend.image import load_image, show_image
from paz.backend.camera import Camera
import redis
import sys


class EmotionDetector(pr.Processor):
    def __init__(self):
        super(EmotionDetector, self).__init__()
        self.detect = HaarCascadeFrontalFace(draw=False)
        self.crop = pr.CropBoxes2D()
        self.classify = MiniXceptionFER()
        print("INIT | EmotionDetector")

    def call(self, image):
        boxes2D = self.detect(image)['boxes2D']
        cropped_images = self.crop(image, boxes2D)
        for cropped_image, box2D in zip(cropped_images, boxes2D):
            box2D.class_name = self.classify(cropped_image)['class_name']
            time.sleep(1)
            class_name = self.classify(cropped_image)['class_name']
            #print(class_name)
            time.sleep(1)
            return class_name

    def get_emotion(self):
        return self

if __name__ == '__main__':
    detect = EmotionDetector()
    parser = argparse.ArgumentParser(description='Real-time face classifier')
    parser.add_argument('-c', '--camera_id', type=int, default=0,
                        help='Camera device ID')
    parser.add_argument('-o', '--offset', type=float, default=0.1,
                        help='Scaled offset to')
    args = parser.parse_args()
    
    camera = Camera(args.camera_id)
    while True:
        image = camera.take_photo()
        emotion_curr = detect(image)
        if emotion_curr is not None:
            print("Current Emotion: " + emotion_curr)
