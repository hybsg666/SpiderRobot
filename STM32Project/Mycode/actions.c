#include "actions.h"
#include "servo.h"
#include "main.h"

// 来自 main.c 的「收到新指令」标志：置 1 时 Move 插值循环立即中断（打断当前动作→复位站立→执行新指令）
extern volatile uint8_t can_rx_flag;

// ============================================================
//  角度常量（请勿随意修改，根据实际舵机调试后统一调整）
// ============================================================

// ----- 小腿角度 -----
#define KNEE_STAND      90    // 垂直站立
#define KNEE_LIFT       180   // 举高（抬腿/展示用）
#define KNEE_WALK_LIFT  115   // 行走时抬腿高度（至少弯30°才离地，100°只弯10°拖地）
#define KNEE_SQUAT      140   // 蹲下时小腿回收
#define KNEE_SPREAD     110   // 展开形态（四肢张开）

// ----- 大腿角度 -----
#define HIP_STAND       40    // 自然站立
#define HIP_SQUAT       60    // 蹲下
#define HIP_SPREAD      70    // 展开形态（四肢张开）
#define HIP_FORWARD     40    // 向前
#define HIP_BACKWARD    140   // 向后

// ============================================================
//  平滑插值基础设施（所有动作共用，消除「顿挫」）
// ============================================================
// 记录各舵机当前角度，供插值从当前值平滑滑到目标值。
// 初始值 = Servo_Init() 的初始姿态：膝盖 90°、大腿 60°（见 servo.c）。
static uint8_t gait_angle[SERVO_MAX] = {90, 90, 90, 90, 60, 60, 60, 60};

// 常用姿态（顺序 = Servo_Channel_t：KNEE_LF..KNEE_LB 再 HIP_LF..HIP_LB）
static const uint8_t POSE_STAND[SERVO_MAX]  = {KNEE_STAND,  KNEE_STAND,  KNEE_STAND,  KNEE_STAND,  HIP_STAND,  HIP_STAND,  HIP_STAND,  HIP_STAND};
static const uint8_t POSE_SQUAT[SERVO_MAX]  = {KNEE_SQUAT,  KNEE_SQUAT,  KNEE_SQUAT,  KNEE_SQUAT,  HIP_SQUAT,  HIP_SQUAT,  HIP_SQUAT,  HIP_SQUAT};
static const uint8_t POSE_SPREAD[SERVO_MAX] = {KNEE_SPREAD, KNEE_SPREAD, KNEE_SPREAD, KNEE_SPREAD, HIP_SPREAD, HIP_SPREAD, HIP_SPREAD, HIP_SPREAD};

// 单个舵机从当前角度平滑滑到目标角度（duration_ms 内，20ms 步进）
static void Move1(Servo_Channel_t ch, uint8_t target, uint16_t duration_ms)
{
    uint8_t s = gait_angle[ch];
    int n = duration_ms / 20; if (n < 1) n = 1;
    uint16_t d = duration_ms / n;
    for (int i = 1; i <= n; i++) {
        if (can_rx_flag) return;                       // 收到新指令 → 立即中断（gait_angle 已停在当前值）
        gait_angle[ch] = (uint8_t)(s + (int)(target - s) * i / n);
        Servo_SetAngle(ch, gait_angle[ch]);
        HAL_Delay(d);
    }
}

// 两个舵机同时从当前角度平滑滑到目标角度
static void Move2(Servo_Channel_t a, uint8_t ta, Servo_Channel_t b, uint8_t tb, uint16_t duration_ms)
{
    uint8_t sa = gait_angle[a];
    uint8_t sb = gait_angle[b];
    int n = duration_ms / 20; if (n < 1) n = 1;
    uint16_t d = duration_ms / n;
    for (int i = 1; i <= n; i++) {
        if (can_rx_flag) return;                       // 收到新指令 → 立即中断
        gait_angle[a] = (uint8_t)(sa + (int)(ta - sa) * i / n);
        gait_angle[b] = (uint8_t)(sb + (int)(tb - sb) * i / n);
        Servo_SetAngle(a, gait_angle[a]);
        Servo_SetAngle(b, gait_angle[b]);
        HAL_Delay(d);
    }
}

// 8 个舵机同时从当前姿态平滑滑到目标姿态
static void MoveAll(const uint8_t target[SERVO_MAX], uint16_t duration_ms)
{
    uint8_t start[SERVO_MAX];
    for (int i = 0; i < SERVO_MAX; i++) start[i] = gait_angle[i];
    int n = duration_ms / 20; if (n < 1) n = 1;
    uint16_t d = duration_ms / n;
    for (int i = 1; i <= n; i++) {
        if (can_rx_flag) return;                       // 收到新指令 → 立即中断
        for (int j = 0; j < SERVO_MAX; j++) {
            gait_angle[j] = (uint8_t)(start[j] + (int)(target[j] - start[j]) * i / n);
            Servo_SetAngle((Servo_Channel_t)j, gait_angle[j]);
        }
        HAL_Delay(d);
    }
}

// ============================================================
//  基础动作
// ============================================================

// 1. 居中（全部 90°，调试用）
void Action_Center(void)
{
    for (int i = 0; i < 4; i++) {
        Servo_SetAngle((Servo_Channel_t)i, 90);
    }
    for (int i = 4; i < 8; i++) {
        Servo_SetAngle((Servo_Channel_t)i, 90);
    }
    HAL_Delay(50);
}

// 2. 自然站立（大腿 40°，小腿 90°，平滑过渡）
void Action_Stand(void)
{
    MoveAll(POSE_STAND, 200);
}

// 3. 蹲下（平滑过渡）
void Action_Squat(void)
{
    MoveAll(POSE_SQUAT, 250);
}

// 4. 展开形态（四肢张开，大腿 70°，小腿 110°，平滑过渡）
void Action_Spread(void)
{
    MoveAll(POSE_SPREAD, 300);
}

// ============================================================
//  单腿控制（展示/调试用）
// ============================================================

// 5. 抬腿（先展开四肢→再指定腿的小腿到 180°，平滑过渡）
void Action_LiftLeg(Servo_Channel_t knee_ch, Servo_Channel_t hip_ch)
{
    (void)hip_ch;                     // 当前只用 knee，参数保留签名
    MoveAll(POSE_SPREAD, 300);        // ① 四肢展开（撑起身体）
    Move1(knee_ch, KNEE_LIFT, 250);   // ② 指定腿抬到 180°
}

// 6. 放下腿（腿放下→四肢收回到站立态，平滑过渡）
void Action_LowerLeg(Servo_Channel_t knee_ch, Servo_Channel_t hip_ch)
{
    (void)hip_ch;
    Move1(knee_ch, KNEE_SPREAD, 150); // ① 小腿回到 110°
    MoveAll(POSE_STAND, 200);         // ② 四肢收回站立
}

// 7. 举手（四肢展开 → 举手 → 放下 → 收回，全程平滑）
void Action_RaiseHand(Servo_Channel_t knee_ch, Servo_Channel_t hip_ch)
{
    (void)hip_ch;                     // 当前动作只用到 knee，hip 参数保留签名
    MoveAll(POSE_SPREAD, 300);        // ① 展开（四肢张开）
    Move1(knee_ch, KNEE_LIFT, 250);   // ② 举手（抬到 180°）
    Move1(knee_ch, KNEE_SPREAD, 150); // ③ 放手（回到 110°）
    MoveAll(POSE_STAND, 200);         // ④ 收回（恢复站立）
}

// 8. 腿向前伸（仅大腿，平滑过渡）
void Action_MoveLegForward(Servo_Channel_t hip_ch)
{
    Move1(hip_ch, HIP_FORWARD, 150);
}

// 9. 腿向后伸（仅大腿，平滑过渡）
void Action_MoveLegBackward(Servo_Channel_t hip_ch)
{
    Move1(hip_ch, HIP_BACKWARD, 150);
}

// ============================================================
//  转向角度常量（对角小跑转向，结构与前进完全一致，只改摆髋方向）
//  左转 = 右腿(RF右前+LF右后)前摆 + 左腿(RB左前+LB左后)后摆
//  右转 = 左转镜像（右腿后摆 + 左腿前摆）
//  ★ 转得不够狠就调下面 8 个数：越偏离站立角40，摆幅越大
//    更狠：90→110；更柔：90→60、0→20
// ============================================================
#define HIP_TURN_LEFT_LF    0     // 左转：LF(右后) 前摆
#define HIP_TURN_LEFT_RF    90    // 左转：RF(右前) 前摆
#define HIP_TURN_LEFT_RB    0     // 左转：RB(左前) 后摆
#define HIP_TURN_LEFT_LB    90    // 左转：LB(左后) 后摆

#define HIP_TURN_RIGHT_LF   90    // 右转：LF(右后) 后摆
#define HIP_TURN_RIGHT_RF   0     // 右转：RF(右前) 后摆
#define HIP_TURN_RIGHT_RB   90    // 右转：RB(左前) 前摆
#define HIP_TURN_RIGHT_LB   0     // 右转：LB(左后) 前摆

#define TURN_PUSH           20    // 蹬地角（和前进的20一致，一般不用动）

// ============================================================
//  前进步态（修正版·对角小跑）
//  实测方向：
//    前腿(LF/RF)：角度增大 → 脚向前伸
//    后腿(RB/LB)：角度减小 → 脚向前伸
//  对角小跑：LF+RB 一组、RF+LB 一组，交替「抬腿 → 前摆 → 落地 → 后蹬」
// ============================================================
#define HIP_FWD_FRONT       85    // 前腿(LF/RF)前摆目标角（向前=增大）
#define HIP_FWD_BACK        20    // 后腿(RB/LB)前摆目标角（向前=减小）
#define HIP_PUSH            40    // 后蹬目标角（回站立位40，推动身体向前）
#define WALK_PHASE_DELAY    150   // 每相位停留时间(ms)，舵机慢就调大

// LF/RB 这两条腿的膝盖装得和 RF/LB 不一样，同样角度抬腿更低，需要再抬高点才离地
#define KNEE_WALK_LIFT_HI   125   // LF/RB 抬腿高度（RF/LB 继续用115）

// 10. 前进（对角小跑，平滑插值版）
void Walk_Forward(int steps)
{
    for (int s = 0; s < steps; s++) {
        // ===== 对角组A（LF+RB）向前迈 =====
        Move2(SERVO_KNEE_LF, KNEE_WALK_LIFT_HI, SERVO_KNEE_RB, KNEE_WALK_LIFT_HI, WALK_PHASE_DELAY / 2);  // ① 抬腿
        Move2(SERVO_HIP_RF, 20, SERVO_HIP_LB, 20, WALK_PHASE_DELAY / 2);                                  // ② 组B 后蹬
        Move2(SERVO_HIP_LF, 5, SERVO_HIP_RB, 75, WALK_PHASE_DELAY);                                      // ③ 组A 前摆
        Move2(SERVO_KNEE_LF, KNEE_STAND, SERVO_KNEE_RB, KNEE_STAND, WALK_PHASE_DELAY);                    // ④ 组A 落地

        // ===== 对角组B（RF+LB）向前迈 =====
        Move2(SERVO_KNEE_RF, KNEE_WALK_LIFT, SERVO_KNEE_LB, KNEE_WALK_LIFT, WALK_PHASE_DELAY / 2);        // ① 抬腿
        Move2(SERVO_HIP_LF, 20, SERVO_HIP_RB, 20, WALK_PHASE_DELAY / 2);                                  // ② 组A 后蹬
        Move2(SERVO_HIP_RF, 75, SERVO_HIP_LB, 5, WALK_PHASE_DELAY);                                      // ③ 组B 前摆
        Move2(SERVO_KNEE_RF, KNEE_STAND, SERVO_KNEE_LB, KNEE_STAND, WALK_PHASE_DELAY);                    // ④ 组B 落地
    }
}

// 11. 后退（对角小跑，平滑插值版，与前进镜像）
void Walk_Backward(int steps)
{
    for (int s = 0; s < steps; s++) {
        // ===== 对角组A（LF+RB）向后迈 =====
        Move2(SERVO_KNEE_LF, KNEE_WALK_LIFT_HI, SERVO_KNEE_RB, KNEE_WALK_LIFT_HI, WALK_PHASE_DELAY / 2);  // ① 抬腿
        Move2(SERVO_HIP_RF, 40, SERVO_HIP_LB, 40, WALK_PHASE_DELAY / 2);                                  // ② 组B 回位40
        Move2(SERVO_HIP_LF, 75, SERVO_HIP_RB, 5, WALK_PHASE_DELAY);                                      // ③ 组A 后摆
        Move2(SERVO_KNEE_LF, KNEE_STAND, SERVO_KNEE_RB, KNEE_STAND, WALK_PHASE_DELAY);                    // ④ 组A 落地

        // ===== 对角组B（RF+LB）向后迈 =====
        Move2(SERVO_KNEE_RF, KNEE_WALK_LIFT, SERVO_KNEE_LB, KNEE_WALK_LIFT, WALK_PHASE_DELAY / 2);        // ① 抬腿
        Move2(SERVO_HIP_LF, 40, SERVO_HIP_RB, 40, WALK_PHASE_DELAY / 2);                                  // ② 组A 回位40
        Move2(SERVO_HIP_RF, 5, SERVO_HIP_LB, 75, WALK_PHASE_DELAY);                                      // ③ 组B 后摆
        Move2(SERVO_KNEE_RF, KNEE_STAND, SERVO_KNEE_LB, KNEE_STAND, WALK_PHASE_DELAY);                    // ④ 组B 落地
    }
}

// 12. 左转（对角小跑转向，结构同前进：抬→蹬地→摆髋→落，只是摆髋方向转）
void Turn_Left(int steps)
{
    for (int s = 0; s < steps; s++) {
        // ===== 对角组A（LF右后 + RB左前） =====
        Move2(SERVO_KNEE_LF, KNEE_WALK_LIFT_HI, SERVO_KNEE_RB, KNEE_WALK_LIFT_HI, WALK_PHASE_DELAY / 2);  // ① 抬A
        Move2(SERVO_HIP_RF, TURN_PUSH, SERVO_HIP_LB, TURN_PUSH, WALK_PHASE_DELAY / 2);                    // ② B蹬地
        Move2(SERVO_HIP_LF, HIP_TURN_LEFT_LF, SERVO_HIP_RB, HIP_TURN_LEFT_RB, WALK_PHASE_DELAY);          // ③ A摆髋
        Move2(SERVO_KNEE_LF, KNEE_STAND, SERVO_KNEE_RB, KNEE_STAND, WALK_PHASE_DELAY);                    // ④ 落A

        // ===== 对角组B（RF右前 + LB左后） =====
        Move2(SERVO_KNEE_RF, KNEE_WALK_LIFT, SERVO_KNEE_LB, KNEE_WALK_LIFT, WALK_PHASE_DELAY / 2);         // ① 抬B
        Move2(SERVO_HIP_LF, TURN_PUSH, SERVO_HIP_RB, TURN_PUSH, WALK_PHASE_DELAY / 2);                    // ② A蹬地
        Move2(SERVO_HIP_RF, HIP_TURN_LEFT_RF, SERVO_HIP_LB, HIP_TURN_LEFT_LB, WALK_PHASE_DELAY);          // ③ B摆髋
        Move2(SERVO_KNEE_RF, KNEE_STAND, SERVO_KNEE_LB, KNEE_STAND, WALK_PHASE_DELAY);                    // ④ 落B
    }
}

// 13. 右转（对角小跑转向，左转镜像）
void Turn_Right(int steps)
{
    for (int s = 0; s < steps; s++) {
        // ===== 对角组A（LF右后 + RB左前） =====
        Move2(SERVO_KNEE_LF, KNEE_WALK_LIFT_HI, SERVO_KNEE_RB, KNEE_WALK_LIFT_HI, WALK_PHASE_DELAY / 2);  // ① 抬A
        Move2(SERVO_HIP_RF, TURN_PUSH, SERVO_HIP_LB, TURN_PUSH, WALK_PHASE_DELAY / 2);                    // ② B蹬地
        Move2(SERVO_HIP_LF, HIP_TURN_RIGHT_LF, SERVO_HIP_RB, HIP_TURN_RIGHT_RB, WALK_PHASE_DELAY);        // ③ A摆髋
        Move2(SERVO_KNEE_LF, KNEE_STAND, SERVO_KNEE_RB, KNEE_STAND, WALK_PHASE_DELAY);                    // ④ 落A

        // ===== 对角组B（RF右前 + LB左后） =====
        Move2(SERVO_KNEE_RF, KNEE_WALK_LIFT, SERVO_KNEE_LB, KNEE_WALK_LIFT, WALK_PHASE_DELAY / 2);         // ① 抬B
        Move2(SERVO_HIP_LF, TURN_PUSH, SERVO_HIP_RB, TURN_PUSH, WALK_PHASE_DELAY / 2);                    // ② A蹬地
        Move2(SERVO_HIP_RF, HIP_TURN_RIGHT_RF, SERVO_HIP_LB, HIP_TURN_RIGHT_LB, WALK_PHASE_DELAY);        // ③ B摆髋
        Move2(SERVO_KNEE_RF, KNEE_STAND, SERVO_KNEE_LB, KNEE_STAND, WALK_PHASE_DELAY);                    // ④ 落B
    }
}
