/**
 * Created by Administrator on 2018/5/29 0029.
 */

/*浏览器加载完成*/
window.onload = function(){
    //clearChatUI();
}
    //关闭浏览器提示信息
window.onbeforeunload = function (e) {
    e = e || window.event;
    var y = e.clientY;
    if (y <= 0//点击浏览器或者选项卡的关闭按钮
        || y >= Math.max(document.body ? document.body.clientHeight : 0, document.documentElement ? document.documentElement.clientHeight : 0)//点击任务栏的关闭按钮
    ) {
        svc_force_Close();
        console.log("关闭浏览器");
    }
    svc_force_Close();
    //谷歌
    console.log("确定要刷新或关闭浏览器窗口？");

}
//服务器连接成功
var isOpen = false;
//服务器错误
var isError = false;
//判断是否是本地
var isself = false;

/*登录发送按钮*/
var sendbtn = document.getElementById('cbutton');
sendbtn.onclick = function(){
    if(isOpen)
    {
        isself = true;
        svc_send();
    }else{
        svc_connectPlatform();
    }
}

/*获取输入框内容*/
var text = document.getElementById('ctext');
function getContent()
{
    return text.value;
}

/*清空聊天框*/
var charconnent = document.getElementById("chatdiv");
function clearChatUI()
{
    while(charconnent.hasChildNodes())
    {
        charconnent.removeChild(charconnent.firstChild);
    }
}


function createChatId1(str)
{
    var charid1 = document.createElement('div');
    charid1.setAttribute('class','chatid1');
    var send = document.createElement('div');
    send.setAttribute('class','send');
    charid1.appendChild(send);
    var content = document.createElement('div');
    content.setAttribute('id','content');
    content.innerHTML = str;
    send.appendChild(content);
    var arrow = document.createElement('div');
    arrow.setAttribute('class','arrow');
    send.appendChild(arrow);

    var header = document.createElement('div');
    header.setAttribute('class','header');
    var img = document.createElement('img');
    img.setAttribute('src','image/monkey.png');
    header.appendChild(img);
    charid1.appendChild(header);
    return charid1;
}

function createChatId2(str)
{
    var charid1 = document.createElement('div');
    charid1.setAttribute('class','chatid2');
    var send = document.createElement('div');
    send.setAttribute('class','send1');
    charid1.appendChild(send);
    var content = document.createElement('div');
    content.setAttribute('id','content1');
    content.innerHTML = str;
    send.appendChild(content);
    var arrow = document.createElement('div');
    arrow.setAttribute('class','arrow1');
    send.appendChild(arrow);

    var header = document.createElement('div');
    header.setAttribute('class','header1');
    var img = document.createElement('img');
    img.setAttribute('src','image/monkey.png');
    header.appendChild(img);
    charid1.appendChild(header);
    return charid1;
}

