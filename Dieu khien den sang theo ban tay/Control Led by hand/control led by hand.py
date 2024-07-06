
import cv2
import mediapipe as mp
import time
import serial
arduino = serial.Serial(port='COM5', baudrate=9600, timeout=0.1)

mp_drawing = mp.solutions.drawing_utils
mp_hand = mp.solutions.hands

hands = mp_hand.Hands(
    model_complexity = 0,
    min_detection_confidence = 0.5,
    min_tracking_confidence = 0.5
)

cap = cv2.VideoCapture(0)


def wite_arduino(x):
    arduino.write(bytes(x))
    time.sleep(0.01)
    # data = arduino.readline()
    # return data    
    

while cap.isOpened():
    success, img = cap.read()
    if not success:
        break

    img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
    result = hands.process(img)
    img = cv2.cvtColor(img, cv2.COLOR_RGB2BGR)
    count = 0
    value = 0
    #ve ban tay
    if result.multi_hand_landmarks:
        myHand = []       
        count = 0
        h, w, c = img.shape # vị trí mới khi đã sửa
        for idx, hand in enumerate(result.multi_hand_landmarks):
            mp_drawing.draw_landmarks(img, hand, mp_hand.HAND_CONNECTIONS)
            for id, lm in enumerate(hand.landmark):              
                # h, w, c = img.shape vị trí cũ trước khi sửa
                myHand.append([int(lm.x * w), int (lm.y * h)]) # x=0 , y =1
            if myHand[8][1] < myHand[6][1]:
                count = count +1        
                # value = wite_arduino(count)
            if myHand[12][1] < myHand[10][1]:
                count = count +1  
                # value = wite_arduino(count)
            if myHand[16][1] < myHand[14][1]:
                count = count +1  
                # value = wite_arduino(count)
            if myHand[20][1] < myHand[18][1]:
                count = count +1  
                # value = wite_arduino(count)
            if myHand[4][0] < myHand[2][0]:
                count = count +1 
                # value = wite_arduino(count) 
                
                
    
    cv2.putText(img, str(count), (50, 50), cv2.FONT_HERSHEY_SIMPLEX, 1, (30, 0, 255), 2, cv2.LINE_AA)

    #hien thi ban tay    
    cv2.imshow("Detect hand", img)
    key = cv2.waitKey(1)
    if key == 27:
        break

cap.release()