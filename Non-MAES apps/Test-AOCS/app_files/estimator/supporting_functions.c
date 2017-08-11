#include "app.h"
/*********************************************************
 * EKF based on Angelo Sabatini paper
 ********************************************************/


/**Initializing all the global variables*/
void init(){
   /*Setting size to the matrices*/
    arm_mat_init_f32(&x,10,1,xData);
    arm_mat_init_f32(&Omega,4,4,OmegaData);
    arm_mat_init_f32(&Phi,10,10,PhiData);
    arm_mat_init_f32(&P,10,10,PData);
    arm_mat_init_f32(&Q,10,10,QData);
    arm_mat_init_f32(&R,6,6,RData);
    arm_mat_init_f32(&K,10,6,KData);
    arm_mat_init_f32(&z,6,1,zData);
    arm_mat_init_f32(&f,6,1,fData);
    arm_mat_init_f32(&F,6,10,FData);
    arm_mat_init_f32(&g,3,1,gData);
    arm_mat_init_f32(&h,3,1,hData);
    arm_mat_init_f32(&Xi,4,3,XiData);
    arm_mat_init_f32(&Eg,3,3,EgData);
    arm_mat_init_f32(&sigmaWa,3,1,sigmaWaData);
    arm_mat_init_f32(&sigmaWm,3,1,sigmaWmData);
    arm_mat_init_f32(&sigmag,3,1,sigmagData);
    arm_mat_init_f32(&sigmaa,3,1,sigmaaData);
    arm_mat_init_f32(&sigmam,3,1,sigmamData);
    arm_mat_init_f32(&gyro_meas,3,1,gyro_measData);
    arm_mat_init_f32(&gyro_meas_next,3,1,gyro_meas_nextData);

    /*Init state : q4=0*/
    x.pData[3]=1;
    /*Init Phi to identity matrix*/
    Phi.pData[0]=1;
    Phi.pData[11]=1;
    Phi.pData[22]=1;
    Phi.pData[33]=1;
    Phi.pData[44]=1;
    Phi.pData[55]=1;
    Phi.pData[66]=1;
    Phi.pData[77]=1;
    Phi.pData[88]=1;
    Phi.pData[99]=1;

    /*Constant values*/
    g.pData[2]=-9.80665;
    h.pData[0]=13.28;
    h.pData[1]=-3.2;
    h.pData[2]=21.2;

    /*Sensor sensitivity*/
    magSens=16;
    accSens=16384/9.80665;
    gyroSens=16.4;

    /*Integration time step*/
    ts=0.001;

    /*Covariance values and matrices*/
    sigmag.pData[0]=0.0168;
    sigmag.pData[1]=0.0135;
    sigmag.pData[2]=0.0135;
    sigmaa.pData[0]=0.0032;
    sigmaa.pData[1]=0.0035;
    sigmaa.pData[2]=0.0039;
    sigmam.pData[0]=0.1634;
    sigmam.pData[1]=0.1705;
    sigmam.pData[2]=0.1831;
    sigmaWa.pData[0]=0.0018;
    sigmaWa.pData[1]=0.0022;
    sigmaWa.pData[2]=0.0024;
    sigmaWm.pData[0]=0.1047;
    sigmaWm.pData[1]=0.1169;
    sigmaWm.pData[2]=0.1119;

    Eg.pData[0]=powf(sigmag.pData[0],2);
    Eg.pData[4]=powf(sigmag.pData[1],2);
    Eg.pData[8]=powf(sigmag.pData[2],2);
    /*Initializing Jacobian matrix (these values won't change)*/
    F.pData[4]=1;
    F.pData[15]=1;
    F.pData[26]=1;
    F.pData[37]=1;
    F.pData[48]=1;
    F.pData[59]=1;

    /*Init Q*/
    updateQ();
    Q.pData[44]=powf(sigmaWa.pData[0],2)*ts;
    Q.pData[55]=powf(sigmaWa.pData[1],2)*ts;
    Q.pData[66]=powf(sigmaWa.pData[2],2)*ts;
    Q.pData[77]=powf(sigmaWm.pData[0],2)*ts;
    Q.pData[88]=powf(sigmaWm.pData[1],2)*ts;
    Q.pData[99]=powf(sigmaWm.pData[2],2)*ts;

   /*Initializing P matrix 10000 times Q0*/
    arm_mat_scale_f32(&Q,10000,&P);
    normalizing();
}

/*Updating Q matrix*/
void updateQ(){
    float32_t q1,q2,q3,q4;
    float32_t temp1Data[12],temp2Data[12],temp3Data[16];
    arm_matrix_instance_f32 temp1,temp2,temp3;
    int i=0;
    int j=0;

    arm_mat_init_f32(&temp1,4,3,temp1Data);
    arm_mat_init_f32(&temp2,3,4,temp2Data);
    arm_mat_init_f32(&temp3,4,4,temp3Data);

    /*Get quaternions*/
    q1=x.pData[0];
    q2=x.pData[1];
    q3=x.pData[2];
    q4=x.pData[3];


    /*Xi update*/
    Xi.pData[0]=q4;
    Xi.pData[1]=-q3;
    Xi.pData[2]=q2;
    Xi.pData[3]=q3;
    Xi.pData[4]=q4;
    Xi.pData[5]=-q1;
    Xi.pData[6]=-q2;
    Xi.pData[7]=q1;
    Xi.pData[8]=q4;
    Xi.pData[9]=-q1;
    Xi.pData[10]=-q2;
    Xi.pData[11]=-q3;

    /*Updating Xi*/
    arm_mat_mult_f32(&Xi, &Eg, &temp1);
    arm_mat_trans_f32(&Xi, &temp2);
    arm_mat_mult_f32(&temp1, &temp2, &temp3);
    arm_mat_scale_f32(&temp3,powf(ts/2,2),&temp3);

    /*Updating Q matrix*/
    i=0;
    while (i<4){
        while (j<4){
            Q.pData[i*10+j]=temp3.pData[i*4+j];
            j++;
        }
        j=0;
        i++;
    }
}

/*Get measurements from sensors and set them in the correct reference frame and correct value by
 * dividing it to the sensor's sensitivity
 */
void get_measurements(){
    gyro_meas_next.pData[0]=s_gyroXYZ.x/gyroSens;
    gyro_meas_next.pData[1]=-1*s_gyroXYZ.y/gyroSens;
    gyro_meas_next.pData[2]=-1*s_gyroXYZ.z/gyroSens;

    z.pData[0]=s_accelXYZ.x/accSens;
    z.pData[1]=-1*s_accelXYZ.y/accSens;
    z.pData[2]=-1*s_accelXYZ.z/accSens;

    z.pData[3]= s_magcompXYZ.x/gyroSens;
    z.pData[4]= s_magcompXYZ.y/gyroSens;
    z.pData[5]= s_magcompXYZ.z/gyroSens;
}

/*Update Phi*/
void updatePhi(){
    float w1,w2,w3;
    float32_t temp1Data[16]={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    arm_matrix_instance_f32 temp1;
    arm_mat_init_f32(&temp1,4,4,temp1Data);
    int i,j;

    w1=gyro_meas.pData[0];
    w2=gyro_meas.pData[1];
    w3=gyro_meas.pData[2];

    /*Updating Omega data, diagonal is zero*/
    Omega.pData[1]=-w3;
    Omega.pData[2]=w2;
    Omega.pData[3]=w1;


    Omega.pData[4]=w3;
    Omega.pData[6]=-w1;
    Omega.pData[7]=w2;


    Omega.pData[8]=-w2;
    Omega.pData[9]=w1;
    Omega.pData[11]=w3;


    Omega.pData[12]=-w1;
    Omega.pData[13]=-w2;
    Omega.pData[14]=-w3;

    arm_mat_scale_f32(&Omega,ts/2,&Omega);

    /*Approx of expm(Omega*ts/2) = eye(4)+ Omega*ts/2*/
    arm_mat_add_f32(&temp1,&Omega,&temp1);

    /*Updating Q matrix*/
    i=0;
    while (i<4){
        while (j<4){
            Phi.pData[i*10+j]=temp1.pData[i*4+j];
            j++;
        }
        j=0;
        i++;
    }

    i=0;
}

/*Get Rotation matrix from quaternion*/
arm_matrix_instance_f32 getRotation(){
    float32_t q1,q2,q3,q4;
    float32_t temp1Data[9];
    arm_matrix_instance_f32 temp1;
    arm_mat_init_f32(&temp1,3,3,temp1Data);

    /*Get quaternions*/
    q1=x.pData[0];
    q2=x.pData[1];
    q3=x.pData[2];
    q4=x.pData[3];


    /*Getting rotation*/
    temp1.pData[0]=powf(q1,2)-powf(q2,2)-powf(q3,2)+powf(q4,2);
    temp1.pData[1]=2*(q1*q2+q3*q4);
    temp1.pData[2]=2*(q1*q3-q2*q4);

    temp1.pData[3]=2*(q1*q2-q3*q4);
    temp1.pData[4]=-powf(q1,2)+powf(q2,2)-powf(q3,2)+powf(q4,2);
    temp1.pData[5]=2*(q2*q3+q4*q1);

    temp1.pData[6]=2*(q1*q3+q2*q4);
    temp1.pData[7]=2*(q2*q3-q4*q1);
    temp1.pData[8]=-powf(q1,2)-powf(q2,2)+powf(q3,2)+powf(q4,2);

    return temp1;
}

/*Update f matrix*/
void updatef(){
    float32_t RotationData[9];
    float32_t tempData[3];
    float32_t biasData[3];
    arm_matrix_instance_f32 Rotation,temp,bias;

    arm_mat_init_f32(&Rotation,3,3,RotationData);
    arm_mat_init_f32(&temp,3,1,tempData);
    arm_mat_init_f32(&bias,3,1,biasData);

    /*Get rotation*/
    Rotation=getRotation();

    /*Getting acc bias*/
    bias.pData[0]=x.pData[4];
    bias.pData[1]=x.pData[5];
    bias.pData[2]=x.pData[6];

    arm_mat_mult_f32(&Rotation, &g, &temp);
    arm_mat_add_f32(&temp,&bias,&temp);

    f.pData[0]=temp.pData[0];
    f.pData[1]=temp.pData[1];
    f.pData[2]=temp.pData[2];

    /*Getting mag bias*/
    bias.pData[0]=x.pData[7];
    bias.pData[1]=x.pData[8];
    bias.pData[2]=x.pData[9];

    arm_mat_mult_f32(&Rotation, &h, &temp);
    arm_mat_add_f32(&temp,&bias,&temp);

    f.pData[3]=temp.pData[0];
    f.pData[4]=temp.pData[1];
    f.pData[5]=temp.pData[2];


}

/*Update Jacobian matrix*/
void updateJacobian(){
    float32_t q1,q2,q3,q4;
    int i=0;
    int j=0;

    /*Get quaternions*/
    q1=x.pData[0];
    q2=x.pData[1];
    q3=x.pData[2];
    q4=x.pData[3];

    /* Cn: Results of partial derivative of column n of dCn/dq*/
    float32_t C1Data[12]={2*q1,-2*q2,-2*q3,2*q4,
                          2*q2,2*q1,-2*q4,-2*q3,
                          2*q3,2*q4,2*q1,2*q2};

    float32_t C2Data[12]={2*q2,2*q1,2*q4,2*q3,
                        -2*q1,2*q2,-2*q3,2*q4,
                        -2*q4,2*q3,2*q2,-2*q1};

    float32_t C3Data[12]={2*q3,-2*q4,2*q1,-2*q2,
                          2*q4,2*q3,2*q2,2*q1,
                        -2*q1,-2*q2,2*q3,2*q4};


    float32_t temp1Data[12],temp2Data[12],temp3Data[12];

    arm_matrix_instance_f32 C1,C2,C3;
    arm_matrix_instance_f32 temp1,temp2,temp3;

    arm_mat_init_f32(&C1,3,4,C1Data);
    arm_mat_init_f32(&C2,3,4,C2Data);
    arm_mat_init_f32(&C3,3,4,C3Data);
    arm_mat_init_f32(&temp1,3,4,temp1Data);
    arm_mat_init_f32(&temp2,3,4,temp2Data);
    arm_mat_init_f32(&temp3,3,4,temp3Data);

    /* Updating Jacobian with partial derivative results
     * F= dc/dq g   eye(3)  0(3x3)
     *    dc/dq h   0(3x3)  eye(3)
     */

    /*g vector*/
    arm_mat_scale_f32(&C1,g.pData[0],&temp1);
    arm_mat_scale_f32(&C2,g.pData[1],&temp2);
    arm_mat_scale_f32(&C3,g.pData[2],&temp3);
    /*Adding all and store in temp1*/
    arm_mat_add_f32(&temp1,&temp2,&temp1);
    arm_mat_add_f32(&temp1,&temp3,&temp1);
    /*Adding to Jacobian matrix*/
    while (i<3){
        while(j<4){
            F.pData[i*10+j]=temp1.pData[i*4+j];
            j++;
        }
        j=0;
        i++;
    }
    /*h vector*/
    arm_mat_scale_f32(&C1,h.pData[0],&temp1);
    arm_mat_scale_f32(&C2,h.pData[1],&temp2);
    arm_mat_scale_f32(&C3,h.pData[2],&temp3);
    /*Adding all and store in temp1*/
    arm_mat_add_f32(&temp1,&temp2,&temp1);
    arm_mat_add_f32(&temp1,&temp3,&temp1);
    /*Adding to Jacobian matrix*/
    i=j=0;
    while (i<3){
        while(j<4){
            F.pData[(i*10)+j+30]=temp1.pData[i*4+j];
            j++;
        }
        j=0;
        i++;
    }
}

/*Normalizing quaternions*/
void normalizing(){
    float32_t mag;

    mag=sqrt(powf(x.pData[0],2)+powf(x.pData[1],2)+powf(x.pData[2],2)+powf(x.pData[3],2));
    x.pData[0]=x.pData[0]/mag;
    x.pData[1]=x.pData[1]/mag;
    x.pData[2]=x.pData[2]/mag;
    x.pData[3]=x.pData[3]/mag;
}

/*update R value*/
void updateR(){


    R.pData[0]=powf(sigmaa.pData[0],2);
    R.pData[7]=powf(sigmaa.pData[1],2);
    R.pData[14]=powf(sigmaa.pData[2],2);

    R.pData[21]=powf(sigmam.pData[0],2);
    R.pData[28]=powf(sigmam.pData[1],2);
    R.pData[35]=powf(sigmam.pData[2],2);
//    float32_t magnitude_a,magnitude_g,magnitude_m,magnitude_h, angle, temp;
//    float32_t RotationData[9],temp1Data[3],temp2Data[3],temp3Data[9];
//    float32_t accData[3],magData[3];
//    arm_matrix_instance_f32 Rotation,temp1,temp2,temp3,acc,mag;
//    arm_mat_init_f32(&Rotation,3,3,RotationData);
//    arm_mat_init_f32(&acc,3,3,accData);
//    arm_mat_init_f32(&mag,3,3,magData);
//    arm_mat_init_f32(&temp1,3,1,temp1Data);
//    arm_mat_init_f32(&temp2,3,1,temp2Data);
//    arm_mat_init_f32(&temp3,3,3,temp3Data);
//
//    acc.pData[0]=z.pData[0];
//    acc.pData[1]=z.pData[1];
//    acc.pData[2]=z.pData[2];
//
//    mag.pData[0]=z.pData[3];
//    mag.pData[1]=z.pData[4];
//    mag.pData[2]=z.pData[5];
//
//    magnitude_a=sqrt(powf(acc.pData[0],2)+powf(acc.pData[1],2)+powf(acc.pData[2],2));
//    magnitude_m=sqrt(powf(mag.pData[0],2)+powf(mag.pData[1],2)+powf(mag.pData[2],2));
//    magnitude_g=sqrt(powf(g.pData[0],2)+powf(g.pData[1],2)+powf(g.pData[2],2));
//    magnitude_h=sqrt(powf(h.pData[0],2)+powf(h.pData[1],2)+powf(h.pData[2],2));
//
//    if (magnitude_a-magnitude_g<0.2){
//        R.pData[0]=powf(sigmaa.pData[0],2);
//        R.pData[7]=powf(sigmaa.pData[1],2);
//        R.pData[14]=powf(sigmaa.pData[2],2);
//    }
//
//    else{ //Set high value
//        R.pData[0]=powf(2,16);
//        R.pData[7]=powf(2,16);
//        R.pData[14]=powf(2,16);
//    }
//    R.pData[21]=powf(sigmam.pData[0],2);
//    R.pData[28]=powf(sigmam.pData[1],2);
//    R.pData[35]=powf(sigmam.pData[2],2);
//
//    Rotation=getRotation();
//    /*Getting transpose of Rotation*/
//    arm_mat_trans_f32(&Rotation,&temp3);
//    arm_mat_mult_f32(&temp3,&acc,&temp1);
//    arm_mat_mult_f32(&temp3,&mag,&temp2);
//
//    arm_dot_prod_f32(temp1.pData,temp2.pData,3,&temp);
//    angle=acos(temp/(magnitude_m*magnitude_a));
//
//    if(abs(magnitude_m-magnitude_h)<10 && abs(angle-158)<10){
//        R.pData[21]=powf(sigmam.pData[0],2);
//        R.pData[28]=powf(sigmam.pData[1],2);
//        R.pData[35]=powf(sigmam.pData[2],2);
//    }
//
//    else{ //Set high value
//        R.pData[21]=powf(2,16);
//        R.pData[28]=powf(2,16);
//        R.pData[35]=powf(2,16);
//    }
}
