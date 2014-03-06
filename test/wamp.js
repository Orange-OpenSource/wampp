var secureCb;
var secureCbLabel;
var wsUri;
var consoleLog;
var connectBut;
var disconnectBut;
var procURI;
var topicURI;
var addParamBut;
var delParamBut;
var sendBut;
var clearLogBut;

function wampTestInit() {

    secureCb = document.getElementById("secureCb");
    secureCb.checked = false;
    secureCb.onclick = toggleTls;

    secureCbLabel = document.getElementById("secureCbLabel")

    wsUri = document.getElementById("wsUri");
    toggleTls();

    connectBut = document.getElementById("connect");
    connectBut.onclick = doConnect;

    disconnectBut = document.getElementById("disconnect");

    procURI = document.getElementById("procURI");
/*    var messages = document.getElementById("messages");
    for (var msg in wampmsgs.call) {
        var option = document.createElement("option");
        option.innerHTML = msg;
        messages.appendChild(option);
    }
    //sendMessage.value = wampmsgs.call["GetNameOwner"];
*/
    addParamBut = document.getElementById("addParam");
    delParamBut = document.getElementById("delParam");
    sendBut = document.getElementById("send");

    topicURI = document.getElementById("topicURI");
    subBut = document.getElementById("subscribe");
    unsubBut = document.getElementById("unsubscribe");

    consoleLog = document.getElementById("consoleLog");

    clearLogBut = document.getElementById("clearLogBut");
    clearLogBut.onclick = clearLog;

    setGuiConnected(false);

}

function addParam() {
    var param = document.createElement("input");
    param.className = "param";
    param.type = "text";
    param.size = 35;
    var params = document.getElementsByClassName("param");
    var insNode;
    if (params.length > 0) {
        insNode = params[params.length-1].nextSibling;
    } else {
        insNode = procURI.nextSibling;
    }
    var parentNode = document.getElementById("wamp-config");
    parentNode.insertBefore(param,insNode);
}

function delParam() {
    var params = document.getElementsByClassName("param");
    if (params) {
        var parentNode = document.getElementById("wamp-config");
        parentNode.removeChild(params[params.length-1]);
    } 
}

function toggleTls() {
    if (secureCb.checked)
    {
        wsUri.value = wsUri.value.replace("ws:", "wss:");
    }
    else
    {
        wsUri.value = wsUri.value.replace ("wss:", "ws:");
    }
}

function doConnect() {
    // Get server URI
    var uri = wsUri.value;
    // Connect using AutoBahn.js
    ab.connect(uri,onOpen,onClose);
}

function logToConsole(message) {
    var pre = document.createElement("p");
    pre.style.wordWrap = "break-word";
    pre.innerHTML = getSecureTag()+message;
    consoleLog.appendChild(pre);

    while (consoleLog.childNodes.length > 50)
    {
        consoleLog.removeChild(consoleLog.firstChild);
    }

    consoleLog.scrollTop = consoleLog.scrollHeight;
}

function onOpen(sess) {
    logToConsole("CONNECTED");
    setGuiConnected(true);
    disconnectBut.onclick = function () {
        sess.close();
    };
    sendBut.onclick = function () {
        var args = new Array();
        args.push(procURI.value);
        var params = document.getElementsByClassName('param');
        for (i=0; i<params.length; i++) {
            args.push(params[i].value);
        }
        // RPC Call (Result stored in a promise)
        var start = new Date().getTime();
        var promise = sess.call.apply(sess,args);
        promise.then(function (res) {
                          logMessage(res);
                          end = new Date().getTime();
                          logToConsole(
                              "Request completed in:" 
                              + (end - start) + "ms");
                     },
                     function (error) {
                         var errStr = "[" + error.uri + " | ";
                         errStr += " " + error.desc + " ]";
                         if (error.detail) {
                             errStr += "[" + error.detail + "]";
                         }
                         logError(errStr);
                     });
    }
    subBut.onclick = function () {
        sess.subscribe(topicURI.value,
                       function (topic,event) {
                           logEvent("[" + topic + "] " + event);
                       });
    }
    unsubBut.onclick = function () {
        sess.unsubscribe(topicURI.value);
    }
}

function onClose(code,reason) {
    logToConsole("DISCONNECTED with code["+code+"]");
    logToConsole("Reason:" + reason);
    setGuiConnected(false);
}

function logMessage(msg) {
    logToConsole('<span style="color: blue;">RESPONSE: ' + msg +'</span>');
}

function logEvent(evt) {
    logToConsole('<span style="color: green;">EVENT: ' + evt +'</span>');
}

function logError(error) {
    logToConsole('<span style="color: red;">ERROR:</span> ' + error);
}

function setGuiConnected(isConnected) {
    wsUri.disabled = isConnected;
    connectBut.disabled = isConnected;
    disconnectBut.disabled = !isConnected;
    var params = document.getElementsByClassName('param');
    for (i=0; i<params.length; i++) {
        params[i].disabled = !isConnected;
    }
    procURI.disabled = !isConnected;
    addParamBut.disabled = !isConnected;
    delParamBut.disabled = !isConnected;
    sendBut.disabled = !isConnected;
    secureCb.disabled = isConnected;
    var labelColor = "black";
    if (isConnected)
    {
        labelColor = "#999999";
    }
    secureCbLabel.style.color = labelColor;

}

function clearLog() {
    while (consoleLog.childNodes.length > 0)
    {
        consoleLog.removeChild(consoleLog.lastChild);
    }
}

function getSecureTag() {
    if (secureCb.checked)
    {
        return '<img src="img/tls-lock.png" width="6px" height="9px"> ';
    }
    else
    {
        return '';
    }
}
