#include <WebServer.h>
#include "app_web.h"
#include "bsp/bsp_can.h"
#include "protocol/can_protocol.h"

// 网页有指令时刷新空闲计时（定义在 main.cpp）
extern void updateActivity();

// Web 服务器对象
WebServer server(80);

// 网页 HTML（直接定义在此文件）
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="zh">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>🕷️ 蜘蛛机器人</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: 'Segoe UI', 'PingFang SC', Roboto, sans-serif;
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            background: radial-gradient(ellipse at center, #0d0d1a 0%, #000000 100%);
            padding: 20px;
            user-select: none;
        }
        .container {
            max-width: 480px;
            width: 100%;
            background: rgba(20, 20, 40, 0.7);
            backdrop-filter: blur(20px);
            -webkit-backdrop-filter: blur(20px);
            border-radius: 48px;
            padding: 32px 24px 40px;
            border: 1px solid rgba(255, 255, 255, 0.06);
            box-shadow: 0 30px 80px rgba(0, 0, 0, 0.8), inset 0 1px 0 rgba(255, 255, 255, 0.04);
        }
        .header { display: flex; align-items: center; justify-content: space-between; margin-bottom: 28px; padding-bottom: 16px; border-bottom: 1px solid rgba(255, 255, 255, 0.04); }
        .header h1 { font-size: 24px; font-weight: 700; color: #fff; letter-spacing: 0.5px; }
        .status-badge { background: rgba(0, 255, 200, 0.12); color: #00ffc8; font-size: 12px; padding: 4px 14px; border-radius: 20px; border: 1px solid rgba(0, 255, 200, 0.15); font-weight: 500; animation: pulse 2.5s ease-in-out infinite; }
        @keyframes pulse { 0%,100%{opacity:0.6;} 50%{opacity:1;} }
        .status-bar { background: rgba(0, 0, 0, 0.35); border-radius: 16px; padding: 12px 18px; margin-bottom: 28px; display: flex; justify-content: space-between; align-items: center; border: 1px solid rgba(255, 255, 255, 0.03); }
        .status-bar .label { color: rgba(255, 255, 255, 0.35); font-size: 13px; font-weight: 500; }
        .status-bar .value { color: #fff; font-size: 14px; font-weight: 600; background: rgba(0, 255, 200, 0.06); padding: 4px 16px; border-radius: 12px; border: 1px solid rgba(0, 255, 200, 0.08); min-width: 60px; text-align: center; }
        .grid { display: grid; grid-template-columns: 1fr 1fr 1fr; gap: 12px; margin-bottom: 20px; }
        .btn { padding: 18px 0; border: none; border-radius: 20px; font-size: 20px; font-weight: 600; cursor: pointer; transition: all 0.1s ease; background: rgba(255, 255, 255, 0.04); color: rgba(255, 255, 255, 0.7); border: 1px solid rgba(255, 255, 255, 0.04); box-shadow: 0 4px 12px rgba(0, 0, 0, 0.2); -webkit-tap-highlight-color: transparent; }
        .btn:active { transform: scale(0.92); }
        .btn .icon { display: block; font-size: 28px; margin-bottom: 2px; }
        .btn .label { display: block; font-size: 11px; font-weight: 400; color: rgba(255, 255, 255, 0.35); }
        .btn-forward { background: rgba(0, 200, 255, 0.08); border-color: rgba(0, 200, 255, 0.12); }
        .btn-forward:active { background: rgba(0, 200, 255, 0.25); }
        .btn-backward { background: rgba(255, 160, 0, 0.08); border-color: rgba(255, 160, 0, 0.12); }
        .btn-backward:active { background: rgba(255, 160, 0, 0.25); }
        .btn-left { background: rgba(0, 255, 200, 0.08); border-color: rgba(0, 255, 200, 0.12); }
        .btn-left:active { background: rgba(0, 255, 200, 0.25); }
        .btn-right { background: rgba(200, 100, 255, 0.08); border-color: rgba(200, 100, 255, 0.12); }
        .btn-right:active { background: rgba(200, 100, 255, 0.25); }
        .btn-stop { background: rgba(255, 60, 80, 0.12); border-color: rgba(255, 60, 80, 0.15); color: #ff6b7a; }
        .btn-stop:active { background: rgba(255, 60, 80, 0.30); }
        .bottom-row { display: grid; grid-template-columns: 1fr 1fr 1fr; gap: 12px; margin-bottom: 12px; }
        .bottom-row2 { display: grid; grid-template-columns: 1fr 1fr 1fr 1fr; gap: 10px; margin-bottom: 12px; }
        .bottom-row3 { display: grid; grid-template-columns: 1fr 1fr 1fr 1fr; gap: 10px; }
        .btn-danger { background: rgba(255, 0, 0, 0.15); border-color: rgba(255, 0, 0, 0.25); color: #ff6b6b; }
        .btn-danger:active { background: rgba(255, 0, 0, 0.30); }
        .btn-secondary { padding: 12px 0; border: none; border-radius: 16px; font-size: 14px; font-weight: 500; cursor: pointer; background: rgba(255, 255, 255, 0.03); color: rgba(255, 255, 255, 0.5); border: 1px solid rgba(255, 255, 255, 0.04); transition: all 0.1s ease; -webkit-tap-highlight-color: transparent; }
        .btn-secondary:active { transform: scale(0.95); background: rgba(255, 255, 255, 0.08); }
        .btn-custom { background: rgba(200, 200, 200, 0.06); border-color: rgba(200, 200, 200, 0.08); }
        .wifi-info { margin-top: 16px; padding: 12px 16px; background: rgba(0, 0, 0, 0.3); border-radius: 12px; border: 1px solid rgba(255, 255, 255, 0.03); text-align: center; }
        .wifi-info span { color: rgba(255, 255, 255, 0.25); font-size: 12px; }
        .wifi-info strong { color: rgba(255, 255, 255, 0.5); font-weight: 500; }
        @media (max-width: 420px) {
            .container { padding: 20px 16px 28px; border-radius: 32px; }
            .btn { padding: 14px 0; font-size: 18px; border-radius: 16px; }
            .btn .icon { font-size: 24px; }
            .header h1 { font-size: 20px; }
            .grid { gap: 8px; }
            .bottom-row { gap: 8px; }
            .bottom-row2 { gap: 6px; }
            .bottom-row3 { gap: 6px; }
            .btn-secondary { font-size: 12px; padding: 10px 0; }
        }
    </style>
</head>
<body>
<div class="container">
    <div class="header"><div><h1>🕷️ 蜘蛛机器人</h1></div><span class="status-badge">● 在线</span></div>
    <div class="status-bar"><span class="label">📡 当前指令</span><span class="value" id="statusText">待命</span></div>
    <div class="grid">
        <div></div><button class="btn btn-forward" onclick="sendCmd('forward')"><span class="icon">⬆️</span><span class="label">前进</span></button><div></div>
        <button class="btn btn-left" onclick="sendCmd('left')"><span class="icon">⬅️</span><span class="label">左转</span></button>
        <button class="btn btn-stop" onclick="sendCmd('stop')"><span class="icon">⏹</span><span class="label">停止</span></button>
        <button class="btn btn-right" onclick="sendCmd('right')"><span class="icon">➡️</span><span class="label">右转</span></button>
        <div></div><button class="btn btn-backward" onclick="sendCmd('backward')"><span class="icon">⬇️</span><span class="label">后退</span></button><div></div>
    </div>
    <div class="bottom-row">
        <button class="btn-secondary" onclick="sendCmd('stand')">🧍 站立</button>
        <button class="btn-secondary" onclick="sendCmd('squat')">🦵 蹲下</button>
        <button class="btn-secondary" onclick="sendCmd('wave')">👋 挥手</button>
    </div>
    <div class="bottom-row2">
        <button class="btn-secondary btn-custom" onclick="sendCmd('cmd14')">⚙️ 动作14</button>
        <button class="btn-secondary btn-custom" onclick="sendCmd('cmd15')">⚙️ 动作15</button>
        <button class="btn-secondary btn-custom" onclick="sendCmd('cmd16')">⚙️ 动作16</button>
        <button class="btn-secondary btn-custom" onclick="sendCmd('cmd17')">⚙️ 动作17</button>
    </div>
    <div class="bottom-row3">
        <button class="btn-secondary btn-danger" onclick="sendCmd('crash')">💥 崩溃测试</button>
        <button class="btn-secondary btn-custom" onclick="sendCmd('reset')">🔄 软复位</button>
        <button class="btn-secondary btn-custom" onclick="sendCmd('status')">📊 状态</button>
        <div></div>
    </div>
    <div class="wifi-info"><span>📶 热点: <strong>SpiderRobot</strong> &nbsp;|&nbsp; 🔑 <strong>12345678</strong></span></div>
</div>
<script>
    const statusEl = document.getElementById('statusText');
    function updateStatus(cmd) {
        const map = {
            'forward':'⬆️ 前进','backward':'⬇️ 后退','left':'⬅️ 左转',
            'right':'➡️ 右转','stop':'⏹ 停止','stand':'🧍 站立',
            'squat':'🦵 蹲下','wave':'👋 挥手',
            'cmd14':'⚙️ 动作14','cmd15':'⚙️ 动作15',
            'cmd16':'⚙️ 动作16','cmd17':'⚙️ 动作17',
            'crash':'💥 崩溃测试','reset':'🔄 软复位','status':'📊 状态'
        };
        statusEl.textContent = map[cmd] || cmd;
    }

    // ===== 修改后的 sendCmd：带 3 秒超时 =====
    function sendCmd(cmd) {
        updateStatus(cmd);

        // 创建 AbortController 实现超时
        const controller = new AbortController();
        const timeoutId = setTimeout(() => {
            controller.abort();
        }, 3000);

        fetch('/cmd?action=' + cmd, {
            signal: controller.signal
        })
        .then(response => response.text())
        .then(data => {
            clearTimeout(timeoutId);
            // 请求成功，无需额外操作
        })
        .catch(err => {
            clearTimeout(timeoutId);
            if (err.name === 'AbortError') {
                // 超时，可能是看门狗复位导致连接断开
                statusEl.textContent = '⏱️ 请求超时，请刷新页面';
                console.log('⏱️ fetch 超时，可能是看门狗复位');
            } else {
                statusEl.textContent = '⚠️ 连接失败';
                console.error(err);
            }
        });
    }

    document.addEventListener('keydown', function(e) {
        const map = {
            'w':'forward','s':'backward','a':'left','d':'right',
            ' ':'stop','1':'stand','2':'squat','3':'wave',
            '4':'cmd14','5':'cmd15','6':'cmd16','7':'cmd17',
            '8':'crash','9':'reset','0':'status'
        };
        const cmd = map[e.key.toLowerCase()];
        if (cmd) { e.preventDefault(); sendCmd(cmd); }
    });
</script>
</body>
</html>
)rawliteral";

// ============================================================
//  处理网页指令（增加 crash / reset / status）
// ============================================================
void handleCommand() {
    updateActivity();   // 网页发来指令 → 刷新活动时间，避免误休眠
    String action = server.arg("action");
    uint8_t canCmd = 0x00;

    // 常规指令
    if (action == "forward") canCmd = CMD_FORWARD;
    else if (action == "backward") canCmd = CMD_BACKWARD;
    else if (action == "left") canCmd = CMD_LEFT;
    else if (action == "right") canCmd = CMD_RIGHT;
    else if (action == "stop") canCmd = CMD_STOP;
    else if (action == "stand") canCmd = CMD_STAND;
    else if (action == "squat") canCmd = CMD_SQUAT;
    else if (action == "wave") canCmd = CMD_WAVE;
    else if (action == "cmd14") canCmd = 0x14;
    else if (action == "cmd15") canCmd = 0x15;
    else if (action == "cmd16") canCmd = 0x16;
    else if (action == "cmd17") canCmd = 0x17;

    // ===== 看门狗测试指令 =====

    else if (action == "crash") {
        Serial0.println("💥 进入死循环，看门狗将在 10 秒后复位！");
        while (1) {
            delay(1000);
            Serial0.print(".");
        }
    }


    else if (action == "reset") {
        Serial0.println("🔄 软件复位...");
        ESP.restart();
    }
    else if (action == "status") {
        Serial0.println("📊 系统状态正常");
        server.send(200, "text/plain", "OK");
        return;
    }

    // 发送 CAN 指令
    bsp_can_send(canCmd, "🌐 网页");
    server.send(200, "text/plain", "OK");
}

// ============================================================
//  Web 服务器初始化
// ============================================================
void app_web_init(void) {
    server.on("/", []() {
        server.send(200, "text/html", index_html);
    });
    server.on("/cmd", handleCommand);
    server.begin();
    Serial0.println("✅ Web 服务器已启动");
}

// ============================================================
//  Web 服务器循环处理
// ============================================================
void app_web_handle(void) {
    server.handleClient();
}
