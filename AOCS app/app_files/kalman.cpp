#include "app.h"

class kalman_filter: public CyclicBehaviour{
    float32_t temp1Data[10],temp2Data[100],temp3Data[100];
    float32_t temp4Data[100],temp5Data[60],temp6Data[60];
    float32_t temp7Data[36],temp8Data[60],temp9Data[6],temp10Data[10];
    float32_t inverseData[36];
    arm_matrix_instance_f32 temp1, temp2,temp3,temp4,temp5;
    arm_matrix_instance_f32 temp6,temp7,temp8,temp9,temp10,inverse;
    int step;
    supporting_functions function;

    void setup(){
        /*Init matrices*/
        arm_mat_init_f32(&temp1,10,1,temp1Data);
        arm_mat_init_f32(&temp2,10,10,temp2Data);
        arm_mat_init_f32(&temp3,10,10,temp3Data);
        arm_mat_init_f32(&temp4,10,10,temp4Data);
        arm_mat_init_f32(&temp5,6,10,temp5Data);
        arm_mat_init_f32(&temp6,10,6,temp6Data);
        arm_mat_init_f32(&temp7,6,6,temp7Data);
        arm_mat_init_f32(&temp8,10,6,temp8Data);
        arm_mat_init_f32(&temp9,6,1,temp9Data);
        arm_mat_init_f32(&temp10,10,1,temp10Data);
        arm_mat_init_f32(&inverse,6,6,inverseData);

        /*Initializing Kalman values*/
        function.init();
        step=GET_MEASUREMENTS;

    }

    void action(){
        switch(step){
            case GET_MEASUREMENTS:
                msg.receive(BIOS_WAIT_FOREVER);
                function.get_measurements();
                step=PRIORI_EST;
                break;

            case PRIORI_EST:
                /*Computes a priori state estimate*/
                arm_mat_mult_f32(&Phi,&x,&temp1);
                x.pData=temp1.pData;
                /*Computes a priori error cov*/
                arm_mat_mult_f32(&Phi,&P,&temp2);//temp2=Phi*P
                arm_mat_trans_f32(&Phi, &temp3);//transpose(Phi)
                arm_mat_mult_f32(&temp2,&temp3,&temp4);// temp4=Phi*P*transpose(Phi)
                arm_mat_add_f32(&temp4,&Q,&P);// P=temp4+Q
                function.normalizing();
                step=KALMAN_GAIN;
                break;

            case KALMAN_GAIN:
                /*Update Jacobian and
                 * covariance matrix of
                 * measurement model R*/
                function.updateR();
                function.updateJacobian();

                /*Computing Kalman gain*/
                arm_mat_mult_f32(&F,&P,&temp5); //temp5=F*P
                arm_mat_trans_f32(&F,&temp6);// temp6=transpose(F)
                arm_mat_mult_f32(&temp5,&temp6,&temp7); //temp7=F*P*trans(F)
                arm_mat_add_f32(&temp7,&R,&temp7); //temp7= temp7+R
                arm_mat_inverse_f32(&temp7,&inverse); //inverse(temp7)
                arm_mat_mult_f32(&P,&temp6,&temp8); // temp8=P*transpose(F)
                arm_mat_mult_f32(&temp8,&inverse,&K);// K=temp8*inverse
                step=POST_EST;
                break;

            case POST_EST:
                /*Update predicted f*/
                function.updatef();

                /*Computes a posteriori state*/
                arm_mat_sub_f32(&z,&f,&temp9); //temp9=z-f
                arm_mat_mult_f32(&K,&temp9,&temp10); //temp1=K*(z-f)
                arm_mat_add_f32(&x,&temp10,&x); //x=x+K*(z-f);
                function.normalizing();

               /*Computes a posteriori error cov*/
                arm_mat_mult_f32(&K,&F,&temp2);//temp2=K*F
                arm_mat_mult_f32(&temp2,&P,&temp3);//temp3=K*F*P
                arm_mat_sub_f32(&P,&temp3,&P);//P=P-K*F*P
                step=UPDATE;
                break;

            case UPDATE:
                function.updatePhi();
                function.updateQ();

                gyro_meas=gyro_meas_next;
                step=GET_MEASUREMENTS;
                msg.send(UART_AID);
                break;
        }
    }
};

/*Class wrapper*/
void kalman(UArg arg0, UArg arg1){
    kalman_filter behaviour;
    behaviour.execute();
}
