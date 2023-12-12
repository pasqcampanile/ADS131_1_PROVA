#ifndef _FILTERS_H_
#define _FILTERS_H_


# define M_PI           3.14159265358979323846  /* pi */

typedef float num;

class LPF {

    private:
        int N;
        num * y_old;
        num alpha;

    public: 
        LPF(int size, num f_cutoff, num Ts){
            N = size;
            y_old = new num[N];
            num tau = 1.00/(2.00*M_PI*f_cutoff) ;
            alpha = Ts / (tau + Ts);
            for(int i =0;i<N;i++) y_old[i]=0.00;
        }

        ~LPF(){
            delete [] y_old;
        }

        void filter(num* u, num* y){
            for(int i =0;i<N;i++){
                y[i] = alpha * u[i] + (1 - alpha) * y_old[i];
                y_old[i] = y[i]; // Update previous output
            }
        }

};


class MA {

    private :
        int N;
        int M_sample;
        num** ma_state;

    public :

        MA(int size, int m_sample){
            N = size; //r
            M_sample = m_sample; //c
            ma_state = new num*[N];
            for(int i =0;i<N;i++) {
                ma_state[i] = new num[M_sample];
            }

            for(int i =0;i<N;i++) {
                for(int j =0;j<M_sample;j++) ma_state[i][j] =0.00;
            }

        }

        ~MA(){
            for(int i =0;i<N;i++) {
                delete [] ma_state[i];
            }
            delete [] ma_state;
        }

        void filter(num* u, num* y){
            for(int i =0;i<N;i++) {
                y[i] =0.00;
                for(int j =0;j<M_sample-2 ;j++){
                    y[i]+= ma_state[i][j];
                    ma_state[i][j] = ma_state[i][j+1] ;
                }
                y[i]+=u[i];
                ma_state[i][M_sample-1] = u[i];

                y[i] = y[i] /num(M_sample);
            }
        
        }

};





#endif //_FILTERS_H_