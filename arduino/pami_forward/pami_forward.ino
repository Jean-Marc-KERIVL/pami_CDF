// ============================================================
//  PAMI Robot — 5s moteurs + servo en boucle infinie
//  Avec protection brown-out + démarrage progressif
// ============================================================
//
//  Câblage:
//    Moteur DROIT  : IN1=D19, IN2=D21
//    Moteur GAUCHE : IN1=D18, IN2=TX2 (GPIO17)
//    Servo SG90    : D26
//    LED interne   : D2

#include "Pins.h"
#include "Motor.h"
#include "Drivetrain.h"
#include "Servo.h"

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// =========== Instances ===========
Motor motorLeft (Pins::LEFT_IN1,  Pins::LEFT_IN2,  /*inverted=*/true);
Motor motorRight(Pins::RIGHT_IN1, Pins::RIGHT_IN2, /*inverted=*/false);
Drivetrain drivetrain(motorLeft, motorRight);

Servo servo(Pins::SERVO, /*pulse_min_us=*/1000, /*pulse_max_us=*/2000);

// Démarrage progressif des moteurs pour limiter le pic de courant
void rampUp(int target_speed, int duration_ms) {
    const int steps = 20;
    int step_delay = duration_ms / steps;
    for (int i = 1; i <= steps; ++i) {
        int s = (target_speed * i) / steps;
        drivetrain.drive(s);
        delay(step_delay);
    }
}

void setup() {
    // 🛡️ Désactive la protection brown-out (évite les reset sur pic de courant)
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    pinMode(Pins::LED, OUTPUT);

    drivetrain.begin();
    servo.begin();
    drivetrain.stop();

    // 3 flashs = boot OK
    for (int i = 0; i < 3; ++i) {
        digitalWrite(Pins::LED, HIGH); delay(120);
        digitalWrite(Pins::LED, LOW);  delay(120);
    }

    delay(1500);   // laisse l'alim se stabiliser

    // === Démarrage progressif des moteurs ===
    digitalWrite(Pins::LED, HIGH);
    rampUp(180, 400);          // 0 -> 180 en 400 ms (4 secondes total avec le delay)
    delay(4600);               // reste à 180 pendant le reste des 5 s
    drivetrain.stop();
    digitalWrite(Pins::LED, LOW);

    delay(500);
}

// Balayage doux du servo: monte ou descend par petits pas
// pour limiter les pics de courant (et donc les brown-out)
void sweep(int from_deg, int to_deg, int step_deg, int step_delay_ms) {
    int step = (to_deg > from_deg) ? step_deg : -step_deg;
    int a = from_deg;
    while ((step > 0 && a < to_deg) || (step < 0 && a > to_deg)) {
        servo.setAngle(a);
        a += step;
        delay(step_delay_ms);
    }
    servo.setAngle(to_deg);
}

void loop() {
    // Monte doucement 0° -> 90° (45 pas de 2°, 30 ms chacun = 1.35 s)
    digitalWrite(Pins::LED, HIGH);
    sweep(0, 90, /*step=*/2, /*step_delay=*/30);
    delay(300);    // pause en haut

    // Descend doucement 90° -> 0°
    digitalWrite(Pins::LED, LOW);
    sweep(90, 0, /*step=*/2, /*step_delay=*/30);
    delay(300);    // pause en bas
}
