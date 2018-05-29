$(document).ready(function() {
    var isTypingMode = false;
    var lapdiv = document.getElementById("lapdiv");
    var monitor = document.getElementById("monitorImage");
    var keyBoard = document.getElementById("keyboardImage");
    var laptop = document.getElementById("lapImage");
    var typingWords = document.getElementById("typer_paragraph");
    // typingWords.style.marginLeft = "190px";
    // var style = monitor.currentStyle || window.getComputedStyle(monitor);
    var isCurrect = [];
    var currectKey = 0;
    var wrongKey = 0;
    var time;
    var speed;
    var lang;
    var isFinished = false;
    // console.log("↹".charCodeAt(0));


    var newRadio1 = document.createElement("input");
    newRadio1.appendChild(document.createTextNode(""));
    newRadio1.id = "python";
    newRadio1.type = "radio";
    newRadio1.name = "lang";
    typingWords.appendChild(newRadio1);
    typingWords.innerHTML += "Python";
    var newRadio2 = document.createElement("input");
    newRadio2.appendChild(document.createTextNode(""));
    newRadio2.id = "java";
    newRadio2.type = "radio";
    newRadio2.name = "lang";
    typingWords.appendChild(newRadio2);
    typingWords.innerHTML += "Java";
    var newRadio3 = document.createElement("input");
    newRadio3.appendChild(document.createTextNode(""));
    newRadio3.id = "c++";
    newRadio3.type = "radio";
    newRadio3.name = "lang";
    newRadio3.checked = "checked";
    typingWords.appendChild(newRadio3);
    typingWords.innerHTML += "C++<br>";

    var startBtn = document.createElement("input");
    startBtn.value = "tap to start typing";
    startBtn.type = "button";
    startBtn.id = "testbutton";
    typingWords.appendChild(startBtn);

    $("#testbutton").click(function() {
        typingWords.innerHTML = "<img id='loadingGif' src='./loadingGifs/loader-meta.gif' style='height:300px; width:400px;'/>";
        // lang = $("input[name=lang]:checked").val();
        // console.log($("input[name=lang]:checked").val());
        // if (document.getElementById('java').checked) {
        //     lang = document.getElementById('java').id;
        //     console.log("java");
        // } else if (document.getElementById('python').checked) {
        //     lang = document.getElementById('python').id;
        //     console.log("pyhton");
        // } else if (document.getElementById('c++').checked) {
        //     lang = document.getElementById('c++').id;
        //     console.log("c++")
        // }
        $.ajax({
            // url: 'http://91.98.76.233:80',
            url: 'http://localhost:8000',
            type: 'POST',
            // contentType: "application/json",
            dataType: 'json',
            data: JSON.stringify({'reqType':'getWords','lang':'java'}),
            beforeSend: function(xhrObj){
                // xhrObj.setRequestHeader("Content-Type","application/json");
                // xhrObj.setRequestHeader("Accept","application/json");
            },
            success: function(data) {
                $("#loadingGif").fadeOut(500);
                setTimeout(function(){
                var i = 0;
                // typingWords.innerHTML = "";
                for (var key in data) {
                    if (data.hasOwnProperty(key)) {
                        key = keyMaker(key);
                        if (i<300 && key.length>0)
                        for (var k=0;k<=key.length;k++){					
                            var newSpan = document.createElement("span");
                            if (k == key.length)
                                newSpan.appendChild(document.createTextNode(" "));
                            else
                                newSpan.appendChild(document.createTextNode(key.charAt(k)));
                            newSpan.id = "char-"+String(i);
                            newSpan.style.margin = "2 0 2 0 px";
                            newSpan.style.borderRadius = "3px";
                            typingWords.appendChild(newSpan);
                            $("#char-"+String(i)).hide();
                            // setTimeout ( function(){
                            $("#char-"+String(i)).fadeIn(i*10);
                            // },100);
                            i++;
                            isCurrect.push(0);
                        }
                    }
                }
                // console.log(i);
                isCurrect[0] = 1;
                highlight(0);
            }, 500);
            },
                error: function() {
                alert('error');
            }        
        });
        // $("#typingWords").fadeIn(500);
        // for (var k=0;k<isCurrect.length;k++)
            // $("#char-"+String(k)).fadeIn();

        var body = document.getElementById("texture1");
        var index = 0;
        document.addEventListener ("keydown",function(e) {
            e = e || window.event;
            var charCode = (typeof e.which == "number") ? e.which : e.keyCode;
            if (!isFinished)
                switch (charCode) {
                    case 8:
                        // console.log("Typed character: " + String.fromCharCode(charCode));
                        if (index>0){
                            isCurrect[index] = 0;
                            isCurrect[index-1] = 1;
                            index--;
                            highlight(index);
                        } else {
                            alert("nemishe ke :)");
                        }
                        break;
                    case 9:
                        $("#typingWords").focus();
                        e.preventDefault();
                        var selectedchar = typingWords.innerText.charCodeAt(index);
                        if (index < typingWords.innerText.length){
                            index++;
                            if (selectedchar == 8633) {
                                    isCurrect[index - 1] = 3;
                                    isCurrect[index] = 1;
                                    highlight(index);
                                } else {
                                    isCurrect[index - 1] = 2;
                                    isCurrect[index] = 1;
                                    highlight(index);
                                    wrongKey++;
                                }
                        }
                        break;
                }
        });
        document.onkeypress = function(e) {
            e = e || window.event;
            var charCode = (typeof e.which == "number") ? e.which : e.keyCode;
            if (!isFinished)
                switch (charCode) {
                    case 13:
                        var selectedchar = typingWords.innerText.charCodeAt(index);
                        if (index < typingWords.innerText.length){
                            index++;
                            if (selectedchar == 9166) {
                                    isCurrect[index - 1] = 3;
                                    isCurrect[index] = 1;
                                    highlight(index);
                                } else {
                                    isCurrect[index - 1] = 2;
                                    isCurrect[index] = 1;
                                    highlight(index);
                                    wrongKey++;
                                }
                        }
                        break;
                    default :
                        if (index == 0){
                            time = performance.now();
                        }
                        var selectedchar = typingWords.innerText.charCodeAt(index);
                        // console.log(charCode);
                        // console.log("Typed character: " + String.fromCharCode(charCode));
                        // console.log("Typed character: " + charCode);
                        // console.log("selected character: " + typingWords.innerText.charCodeAt(index));
                        if (index < typingWords.innerText.length){
                            index++;
                            if (selectedchar == charCode) {
                                isCurrect[index - 1] = 3;
                                isCurrect[index] = 1;
                                highlight(index);
                            } else {
                                isCurrect[index - 1] = 2;
                                isCurrect[index] = 1;
                                highlight(index);
                                wrongKey++;
                            }
                        } else {
                            for (var k=0;k<typingWords.innerText.length;k++){
                                if (isCurrect[k]==3)
                                    currectKey++;
                            }
                            time = (performance.now() - time)/60000;
                            speed = (currectKey)/(5*time);
                            time = time * 60;
                            // lang = String($("input[name=lang]:checked").val());
                            result(speed,time);
                            // alert("tammmaaaam!\ntime: " + String(time*60)+"\nwrongKey: " + String(wrongKey)+ "\ncurrectKry: " + String(currectKey) + "\nspeed: " + String(speed) + "wpm");
                            isFinished = true;
                        }
                        break;
                }
        };

        function result(speed,time){
            // console.log("inja");
            // typingWords.style.marginLeft = "0px";
            typingWords.innerHTML = "";
            typingWords.innerHTML = typingWords.innerHTML + "<span class='result' style='font-size : 15px;'>Typing Speed (WPM)</span>";
            typingWords.innerHTML = typingWords.innerHTML + "<span class='result' style='font-size : 65px;'>" + String(Math.floor(speed)) + "</span>";
            typingWords.innerHTML = typingWords.innerHTML + "<span class='result' style='font-size : 15px;'>Time (s)</span>";
            typingWords.innerHTML = typingWords.innerHTML + "<span class='result' style='font-size : 40px;'>" + String(Math.round(time * 100) / 100) + "</span>";
            typingWords.innerHTML = typingWords.innerHTML + "<span class='result' style='font-size : 15px;'>KeyStrokes</span>";
            typingWords.innerHTML = typingWords.innerHTML + "<span class='result' style='font-size : 35px;'>" + String(currectKey + wrongKey) + " </span>";
            typingWords.innerHTML = typingWords.innerHTML + "<span class='result' style='color: #39FF14 ; font-size : 20px;'> currect : " + String(currectKey) + "</span>";
            typingWords.innerHTML = typingWords.innerHTML + "<span class='result' style='color: #FA8072 ; font-size : 20px;'> wrong : " + String(wrongKey) + "</span>";
            $.post({
                // url: 'http://91.98.76.233:80',
                url: 'http://localhost:80',
                contentType: "application/json",
                dataType: 'text',
                data : JSON.stringify({'reqType':'typingResultSubmit','time': time , 'speed': speed , 'wrongKey': wrongKey , 'currectKey': currectKey , 'lang' : 'java' })
                // data: { 'time': time , 'speed': String(speed) , 'wrongKey': String(wrongKey) , 'currectKey': String(currectKey) }
                // data : { "resid" : "are :)" }
                // success: function (response) {
                //     // you will get response from your php page (what you echo or print)                 
            
                //  },
                //  error: function(jqXHR, textStatus, errorThrown) {
                //     console.log(textStatus, errorThrown);
                //  }
            });
        }


        function highlight(index1) {
            var text = "";
            for (var k=index1-1;k<=index1+1;k++) {
                // console.log("k:"+ String(k));
                // console.log("index:" + String(index));
                // console.log("index1:" + String(index1));
                // console.log("char-" + String(k));
                switch (isCurrect[k]) {
                    case 0:	
                        document.getElementById("char-" + String(k)).style.backgroundColor = "#ffffff";
                        break;
                    case 1:
                        document.getElementById("char-" + String(k)).style.backgroundColor = "#C0C0C0";
                        break;
                    case 2:
                        document.getElementById("char-" + String(k)).style.backgroundColor = "#FA8072";
                        break;
                    case 3:
                        document.getElementById("char-" + String(k)).style.backgroundColor = "#39FF14";
                        break;
                }
            }
        }

        function keyMaker(key){
            var output="";
            for (var k =0; k<key.length;k++)
                if (key.charAt(k) != '\n' && key.charAt(k)!="\t")
                    output += key.charAt(k);
                else if (key.charAt(k) == '\n')
                    output += "⏎";
                else if (key.charAt(k) == '\t')
                    output += "↹";

            return output;
        }

        function clickHandler() {
            if (isTypingMode == false){	
                isTypingMode = true;
                laptop.style.opacity = "0";
                    
                keyBoard.style.transform = "perspective( 1600px ) rotateX( 0deg )"; 
                keyBoard.style.marginTop = "-61px";
                keyBoard.style.width = "650px";
                keyBoard.style.marginLeft = "59px";
                
                monitor.style.transform = "perspective( 1400px ) rotateX( -20deg )";	
                monitor.style.transformOrigin =" 50% 50%";	
                monitor.style.marginTop = "-30px";
                
                // typingWords.style.transform = "translate(-50%, -50%)";
                typingWords.style.transform = "perspective( 1400px ) rotateX( -20deg )";	
            }
            else {
                isTypingMode = false;
                
                keyBoard.style.transform = "perspective( 1600px ) rotateX( 80deg )";
                keyBoard.style.transformOrigin = "50% 0%";
                keyBoard.style.transition = "1s all linear";	
                keyBoard.style.marginTop = "-25px";
                keyBoard.style.marginLeft = "62px";
                keyBoard.style.width = "675px";
                
                monitor.style.transform = "perspective( 1400px ) rotateX( 0deg )";	
                monitor.style.transition = "1s all linear";	
                monitor.style.width= "685px";
                monitor.style.marginLeft= "57px"; 
                monitor.style.marginTop = "-10px";
        
                // typingWords.style.transform = "translate(-50%, -50%)";
                typingWords.style.transform = "perspective( 1400px ) rotateX( 0deg )";	
                window.setTimeout(changeOpacity, 990);
            }
        }
        
        lapdiv.addEventListener("click", clickHandler);			 
        
        function changeOpacity(){
            laptop.style.opacity = "1";
        }
    });
});
