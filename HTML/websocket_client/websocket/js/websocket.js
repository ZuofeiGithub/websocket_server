/**
 * Created by Administrator on 2018/5/30 0030.
 */
/*连接服务器响应事件*/
function svc_connectPlatform() {
    //var wsServer = 'ws://192.168.52.128:4567/';
    //var wsServer = 'ws://127.0.0.1:4567/';
    var wsServer = 'ws://127.0.0.1:4567/';
    try {
        if(!isOpen) {
            svc_websocket = new WebSocket(wsServer);
        }else{
            console.log("您已经连接了服务器...\n");
        }
    } catch (evt) {
        console.log("new WebSocket error:" + evt.data);
        svc_websocket = null;
        if (typeof(connCb) != "undefined" && connCb != null)
            connCb("-1", "connect error!");
        return;
    }
    svc_websocket.onopen = svc_onOpen;
    svc_websocket.onclose = svc_onClose;
    svc_websocket.onmessage = svc_onMessage;
    svc_websocket.onerror = svc_onError;
}
function svc_Close() {
    svc_websocket.onclose(1);
}
function svc_force_Close() {
    svc_websocket.close();
}
function svc_onOpen(evt) {
    isOpen = true;
    isError = false;
    console.log("Connected to WebSocket server.");
    if(isOpen){
        sendbtn.value = '发送';
        text.removeAttribute('disabled');
    }
}
function svc_onClose(evt) {
    if(isOpen)
    {
        client_exit();
        isOpen = false;
        sendbtn.value = '登录';
        text.setAttribute("disabled","disabled")
        console.log("Disconnected");
    }
}
function svc_onMessage(evt) {
    var blob = evt.data;
    var reader = new FileReader();
    reader.readAsText(blob, 'utf-8');
    reader.onload = function (e) {
        console.info(reader.result);
        if(isself)
        {
            charconnent.appendChild(createChatId1(reader.result));
        }else{
            charconnent.appendChild(createChatId2(reader.result));
        }
        isself = false;
        charconnent.scrollTop = charconnent.scrollHeight;
    }
}
function svc_onError(evt) {
    isError = true;
}


function svc_send() {
    var content = getContent();
    if(content.length > 0)
    {
        if(typeof svc_websocket !="undefined") {
            if (svc_websocket.readyState == WebSocket.OPEN && isOpen == true) {
                svc_websocket.send(content);
            } else {
                if(!isError) {
                    console.log("send failed. websocket not open. please check.");
                }
            }
        }else{
            console.log("请先连接服务器...\n");
        }
    }else{
        console.log("请输入文字内容");
    }

}

function client_exit() {
    if(typeof svc_websocket !="undefined") {
        if (svc_websocket.readyState == WebSocket.OPEN && isOpen == true) {
            svc_websocket.send("exit\0");
        } else {
            isError = true;
        }
    }else{
        //console.log("请先连接服务器...\n");
        alert("请先连接服务器...\n");
    }
}