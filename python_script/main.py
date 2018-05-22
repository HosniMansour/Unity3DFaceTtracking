import pandas as pd
import numpy as np

file = pd.read_csv("data.csv",skipinitialspace=True)



columns= ['pose_Rx','pose_Ry','pose_Rz','gaze_0_x','gaze_0_y','gaze_0_z','gaze_1_x','gaze_1_y','gaze_1_z']

x = file.loc[:,columns].values


head1avg=np.zeros(15);
head2avg=np.zeros(15);
head3avg=np.zeros(15);

head1=np.median(head1avg);
head2=np.median(head2avg);
head3=np.median(head3avg);

for i in range(x.shape[0]):
    name = ".\\co\\coeff" + '{0:04}'.format(i+1) + ".txt"
    
    # 50 '0' lwala :p
    
    tab = np.zeros(50);
    
    # head
    head1avg[:-1] = head1avg[1:]; head1avg[-1] = x[i][0];
    head2avg[:-1] = head2avg[1:]; head2avg[-1] = x[i][1];
    head3avg[:-1] = head3avg[1:]; head3avg[-1] = x[i][2];
    
    head1 = np.median(head1avg);
    head2 = np.median(head2avg);
    head3 = np.median(head3avg);

    tab = np.append(tab,head1)
    tab = np.append(tab,head2)
    tab = np.append(tab,head3)
    
    # pose
    
    tab = np.append(tab,x[i][3])
    tab = np.append(tab,x[i][4])
    tab = np.append(tab,x[i][5])
    tab = np.append(tab,x[i][6])
    tab = np.append(tab,x[i][7])
    tab = np.append(tab,x[i][8])
    np.savetxt(name,tab)
    
