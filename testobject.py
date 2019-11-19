import numpy as np
import os
import six.moves.urllib as urllib
import sys
import tarfile
import tensorflow as tf
import zipfile
import random
from collections import defaultdict
from io import StringIO
from matplotlib import pyplot as plt
from PIL import Image
import cv2
import serial
ser = serial.Serial("COM11",9600)
ser2=serial.Serial("COM12",9600)   
cap = cv2.VideoCapture(0)
# This is needed since the notebook is stored in the object_detection folder.
sys.path.append("..")


# ## Object detection imports
# Here are the imports from the object detection module.


from utils import label_map_util

from utils import visualization_utils as vis_util


# # Model preparation 

# ## Variables
# 
# Any model exported using the `export_inference_graph.py` tool can be loaded here simply by changing `PATH_TO_CKPT` to point to a new .pb file.  
# 
# By default we use an "SSD with Mobilenet" model here. See the [detection model zoo](https://github.com/tensorflow/models/blob/master/object_detection/g3doc/detection_model_zoo.md) for a list of other models that can be run out-of-the-box with varying speeds and accuracies.


# What model to download.
MODEL_NAME = 'ssd_mobilenet_v1_coco_11_06_2017'
MODEL_FILE = MODEL_NAME + '.tar.gz'
DOWNLOAD_BASE = 'http://download.tensorflow.org/models/object_detection/'

# Path to frozen detection graph. This is the actual model that is used for the object detection.
PATH_TO_CKPT = MODEL_NAME + '/frozen_inference_graph.pb'

# List of the strings that is used to add correct label for each box.
PATH_TO_LABELS = os.path.join('data', 'mscoco_label_map.pbtxt')

NUM_CLASSES = 8


# ## Download Model



opener = urllib.request.URLopener()
opener.retrieve(DOWNLOAD_BASE + MODEL_FILE, MODEL_FILE)
tar_file = tarfile.open(MODEL_FILE)
for file in tar_file.getmembers():
  file_name = os.path.basename(file.name)
  if 'frozen_inference_graph.pb' in file_name:
    tar_file.extract(file, os.getcwd())


# ## Load a (frozen) Tensorflow model into memory.


detection_graph = tf.Graph()
with detection_graph.as_default():
  od_graph_def = tf.GraphDef()
  with tf.gfile.GFile(PATH_TO_CKPT, 'rb') as fid:
    serialized_graph = fid.read()
    od_graph_def.ParseFromString(serialized_graph)
    tf.import_graph_def(od_graph_def, name='')


# ## Loading label map
# Label maps map indices to category names, so that when our convolution network predicts `5`, we know that this corresponds to `airplane`.  Here we use internal utility functions, but anything that returns a dictionary mapping integers to appropriate string labels would be fine



label_map = label_map_util.load_labelmap(PATH_TO_LABELS)
categories = label_map_util.convert_label_map_to_categories(label_map, max_num_classes=NUM_CLASSES, use_display_name=True)
category_index = label_map_util.create_category_index(categories)


# ## Helper code



def load_image_into_numpy_array(image):
  (im_width, im_height) = image.size
  return np.array(image.getdata()).reshape(
      (im_height, im_width, 3)).astype(np.uint8)

## width function for different   
def r_width(class_id):
  if class_id==3 :
   real_width=1.8 
  elif class_id==6:
   real_width=2.5
  else:
   real_width=1.8
  
  return real_width   
     
     
    

# # Detection


# For the sake of simplicity we will use only 2 images:
# image1.jpg
# image2.jpg
# If you want to test the code with your images, just add path to the images to the TEST_IMAGE_PATHS.
PATH_TO_TEST_IMAGES_DIR = 'test_images'
TEST_IMAGE_PATHS = [ os.path.join(PATH_TO_TEST_IMAGES_DIR, 'image{}.jpg'.format(i)) for i in range(1, 3) ]

# Size, in inches, of the output images.
IMAGE_SIZE = (12, 8)




with detection_graph.as_default():
  with tf.Session(graph=detection_graph) as sess:
    while True:
      ret, image_np = cap.read()
      #getVal = ser.readline().strip()
      if ser2.inWaiting() > 0:  
       object_distance= ser2.readline(-1).strip()
       ser2.flushInput()
	  if ser2.inWaiting() > 0:  
       object_speed= ser2.readline(0).strip()
       ser2.flushInput()
       if ser.inWaiting() > 0:  
        getVal= ser.readline().strip()
        ser.flushInput()
       
      cv2.putText(image_np,'Car speed:',(10,50), cv2.FONT_HERSHEY_SIMPLEX, 1.0, (255,0,0), 2)
      cv2.putText(image_np,getVal.decode('ascii'),(180,50), cv2.FONT_HERSHEY_SIMPLEX, 1.0, (255,0,0), 2)
      cv2.putText(image_np,'object distance:',(280,50), cv2.FONT_HERSHEY_SIMPLEX, 1.0, (0,255,0), 2)
      cv2.putText(image_np,object_distance.decode('ascii'),(530,50), cv2.FONT_HERSHEY_SIMPLEX, 1.0, (0,255,0), 2
      )
      # Expand dimensions since the model expects images to have shape: [1, None, None, 3]
      image_np_expanded = np.expand_dims(image_np, axis=0)
      image_tensor = detection_graph.get_tensor_by_name('image_tensor:0')
      # Each box represents a part of the image where a particular object was detected.
      boxes = detection_graph.get_tensor_by_name('detection_boxes:0')
      # Each score represent how level of confidence for each of the objects.
      # Score is shown on the result image, together with the class label.
      scores = detection_graph.get_tensor_by_name('detection_scores:0')
      classes = detection_graph.get_tensor_by_name('detection_classes:0')
      num_detections = detection_graph.get_tensor_by_name('num_detections:0')
      # Actual detection.
      (boxes, scores, classes, num_detections) = sess.run(
          [boxes, scores, classes, num_detections],
          feed_dict={image_tensor: image_np_expanded})
      # Visualization of the results of a detection.
      vis_util.visualize_boxes_and_labels_on_image_array(
          image_np,
          np.squeeze(boxes),
          np.squeeze(classes).astype(np.int32),
          np.squeeze(scores),
          category_index,
          use_normalized_coordinates=True,
          line_thickness=8)
         
      for i,b in enumerate(boxes[0]):
        #                 car                    bus                  truck
        if classes[0][i] == 3 or classes[0][i] == 6 or classes[0][i] == 8:
          if scores[0][i] >= 0.5:
            rw=r_width(classes[0][i])
            mid_x = (boxes[0][i][1]+boxes[0][i][3])/2
            mid_y = (boxes[0][i][0]+boxes[0][i][2])/2
            width=((boxes[0][i][3] - boxes[0][i][1])*854)
            apx_distance =round((1260*rw)/width)
            #apx_distance=distance(scores[0][i],width)
            v=int(getVal.decode('ascii'))
            v_object=int(object_speed.decode('ascii'))
            cv2.putText(image_np, '{}'.format(apx_distance), (int(mid_x*640),int(mid_y*480)), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255,255,255), 2)
            if apx_distance-0.56*v-1.667<0 and v_object>0:
              cv2.putText(image_np, 'WARNING!!!',(int(mid_x*640)-50,int(mid_y*480)-50) , cv2.FONT_HERSHEY_SIMPLEX, 1.0, (0,0,255), 3)   
      cv2.imshow('object detection', cv2.resize(image_np, (854,480)))
     

      if cv2.waitKey(25) & 0xFF == ord('q'):
        cap.release()
        cv2.destroyAllWindows()
        break