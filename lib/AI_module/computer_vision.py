import os
import urllib.request
import cv2

# ---------------------------------------------------------
# 1. TỰ ĐỘNG TẢI FILE CASCADE NẾU THIẾU TRONG OPENCV (PYTHON 3.14)
# ---------------------------------------------------------
FACE_XML = "haarcascade_frontalface_default.xml"
EYE_XML = "haarcascade_eye.xml"

FACE_URL = "https://raw.githubusercontent.com/opencv/opencv/master/data/haarcascades/haarcascade_frontalface_default.xml"
EYE_URL = "https://raw.githubusercontent.com/opencv/opencv/master/data/haarcascades/haarcascade_eye.xml"

script_dir = os.path.dirname(os.path.abspath(__file__))
face_xml_path = os.path.join(script_dir, FACE_XML)
eye_xml_path = os.path.join(script_dir, EYE_XML)

if not os.path.exists(face_xml_path):
  print("Đang tải file haarcascade_frontalface_default.xml...")
  urllib.request.urlretrieve(FACE_URL, face_xml_path)

if not os.path.exists(eye_xml_path):
  print("Đang tải file haarcascade_eye.xml...")
  urllib.request.urlretrieve(EYE_URL, eye_xml_path)

# Nạp bộ phân loại từ file đã tải
face_cascade = cv2.CascadeClassifier(face_xml_path)
eye_cascade = cv2.CascadeClassifier(eye_xml_path)

if face_cascade.empty() or eye_cascade.empty():
  print("Lỗi: Không thể nạp file cascade XML.")
  exit(1)

# ---------------------------------------------------------
# 2. KHỞI TẠO WEBCAM & CÁC THAM SỐ CẢNH BÁO
# ---------------------------------------------------------
cap = cv2.VideoCapture(0)

DROWSY_FRAMES_THRESH = 20  # Ngưỡng số frame nhắm mắt liên tục -> Báo ngủ gật
DISTRACTED_FRAMES_THRESH = (
    25  # Ngưỡng số frame không thấy mặt liên tục -> Báo mất tập trung
)

drowsy_counter = 0
distracted_counter = 0

print("Chương trình đang chạy. Nhấn phím 'q' hoặc 'ESC' để thoát.")

while cap.isOpened():
  ret, frame = cap.read()
  if not ret:
    print("Không thể kết nối webcam.")
    break

  frame = cv2.flip(frame, 1)
  gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

  # 1. Phát hiện khuôn mặt
  faces = face_cascade.detectMultiScale(
      gray, scaleFactor=1.1, minNeighbors=5, minSize=(100, 100)
  )

  if len(faces) == 0:
    # Không thấy khuôn mặt (mất tập trung / ngoảnh đi nơi khác)
    distracted_counter += 1
    drowsy_counter = 0
    if distracted_counter >= DISTRACTED_FRAMES_THRESH:
      cv2.putText(
          frame,
          "CANH BAO: MAT TAP TRUNG!",
          (30, 80),
          cv2.FONT_HERSHEY_SIMPLEX,
          1,
          (0, 165, 255),
          3,
      )
  else:
    distracted_counter = 0
    for x, y, w, h in faces:
      cv2.rectangle(frame, (x, y), (x + w, y + h), (255, 0, 0), 2)

      # 2. Phát hiện mắt trong vùng khuôn mặt
      roi_gray = gray[y : y + h, x : x + w]
      roi_color = frame[y : y + h, x : x + w]

      eyes = eye_cascade.detectMultiScale(
          roi_gray, scaleFactor=1.1, minNeighbors=10, minSize=(30, 30)
      )

      if len(eyes) == 0:
        # Có mặt nhưng không thấy mắt (nhắm mắt / ngủ gật)
        drowsy_counter += 1
        if drowsy_counter >= DROWSY_FRAMES_THRESH:
          cv2.putText(
              frame,
              "CANH BAO: NGU GAT!",
              (30, 80),
              cv2.FONT_HERSHEY_SIMPLEX,
              1,
              (0, 0, 255),
              3,
          )
      else:
        drowsy_counter = 0
        for ex, ey, ew, eh in eyes:
          cv2.rectangle(
              roi_color, (ex, ey), (ex + ew, ey + eh), (0, 255, 0), 2
          )

  cv2.imshow("Drowsiness & Focus Monitor", frame)

  key = cv2.waitKey(1) & 0xFF   
  if key == ord("q") or key == 27:
    break

cap.release()
cv2.destroyAllWindows()