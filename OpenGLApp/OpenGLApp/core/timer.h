#ifndef TIMER_H
#define TIMER_H

#include <functional>

class Timer {
protected:
    float duration;
    float elapsed;
    bool repeatable;
    bool finished;
    std::function<void()> callbackFunction;
    bool paused;

public:
    //durata del timer, nome funzione da chiamare alla fine del timer, bool che se =true fa ripartire il timer appena finisce
    Timer (float duration, std::function<void()> callbackFunction, bool repeatable)
        : duration(duration), elapsed(0.0f), repeatable(repeatable), finished(false), callbackFunction(callbackFunction), paused(false) {
        //printf("timer di %f secondi iniziato\n", duration);
    }
    


    //per modificare durata, il fatto che si ripeta o meno oppure la funzione di callback
    void changeDuration(float newDuration) {
        duration = newDuration;
        // printf("nuova durata %f\n", newDuration);
    }

    void changeRepeatable(bool newRepeatable) {
        repeatable = newRepeatable;
        
        //if (newRepeatable == true) reset();
    }

    void changeCallback(std::function<void()> newCallbackFunction) {
        callbackFunction = newCallbackFunction;
    }

    //per mettere in pausa e far ripartire timer (se implementiamo un menu di pausa puo' servire)
    void pause() {
        paused = true;
    }
    void resume() {
        paused = false;
        // printf("timer ripartito\n");
    }


    //faccio ricominciare il timer
    void reset() {
        elapsed = 0.0f;
        finished = false;
    }

    //qui passo il delta time per aggiornare elapsed e confrontarlo con duration fino a che non finisce il timer
    void updateTimer(float deltaTime) {
        //se il timer e' in pausa non prosegue il count finche' non si fa resume
        if (paused == true)
            return;

        if (finished == true) {
            if (repeatable == true) {
                finished = false;
                reset();
            }
                return;
        }
            

        elapsed += deltaTime;
        // printf("timer va ancora %f\n", elapsed);
        if (elapsed >= duration) {
            finished = true;
            // printf("timer di %f secondi terminato\n", duration);
            if (callbackFunction)
                callbackFunction();
        }
    }
};

#endif