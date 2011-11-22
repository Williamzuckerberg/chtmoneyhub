﻿/*************************************/
/*input、checkbox、验证效果/
/*************************************/

/*effect*/
Hub.effect = (function(){
    /*private attribute*/
	var inputImage = {
	    "over": "",
		"out": "",
		"bgX": 0,
		"bgY": 0
	}
	
	var regTest = {
	    "mail": /^([a-zA-Z0-9_\.\-\s])+\@(([a-zA-Z0-9\-])+\.)+([a-zA-Z0-9]{2,4})+$/,
	    "whiteMail": /.(@163.com|@126.com|@188.com|@139.com|@qq.com|@sohu.com|@sina.com|@sina.cn|@yahoo.cn|@yahoo.com.cn|@hotmail.com|@gmail.com|@wo.com.cn|@189.cn)/,
	    "password": /^[a-zA-Z0-9_~`!@#$%^&*\(\)\'\'\:\>\<\.\,\+\-\?\/\t\r\n\s]{6,20}$/
	}
	
	/*input dom
	   <div>
	       <input/>
		   <div>
		      <img/>
		   </div>
	   </div>
	*/
	/*private method*/
	function inputBlur(){
	   if (this.id.indexOf("login") != -1){
		   if (Hub.selectOption.getSBValue() != 1){
			  try{
				 Hub.dom.getById(Hub.selectOption.getSBValue()).style.display = "none";
			  }catch(e){
				 //alert(e);
			  }
		   }
	   }
	   
	   if (this.value == ""){
	       if (this.type == "text"){
		      this.value = this.defaultValue
		      this.style.color = "#C3C3C3";
		   }else if (this.type == "password"){
		      Hub.dom.prevE(this).value = Hub.dom.prevE(this).defaultValue;
			  Hub.dom.prevE(this).style.color = "#C3C3C3";
		      Hub.dom.prevE(this).style.display = "inline-block";
		      this.style.display = "none";
		   }
		   validate(this.id, "1"); 
	   }else{
	       validate(this.id);   
	   }
	   Hub.dom.parent(this).style.background = "url(" + inputImage.out + ") no-repeat " + inputImage.bgX + "px " + inputImage.bgY + "px";
	}
	
    function inputFocus(){
	   if (this.defaultValue == this.value) {
		   this.value = "";
		   if (this.getAttribute("psChange") == "1"){
			   Hub.dom.nextE(this).style.display = "inline-block";
			   Hub.dom.nextE(this).focus();
			   this.style.display = "none";
		   }
	   }
	   Hub.dom.parent(this).style.background = "url(" + inputImage.over + ") no-repeat";
	   this.style.color = "#1E1E1E";
	}

	
	function validate(id, flag){
	     var validateId = id + "-validate",
		     validateTipId = id + "-tip",
		     validateHTML = Hub.dom.getById(validateId);
	     switch(id){
	          case "login-mail":		  
			      validateHTML.innerHTML = (flag != undefined) ? "" : validateMail(id);
			      break;
			  //case "login-password":
			      //validateHTML.innerHTML = (flag != undefined) ? "" : validatePs(id);
			 // break;
			  case "register-mail":  
			      validateHTML.innerHTML = (flag != undefined) ? "" : validateMail(id);
				  registerTip(validateHTML.innerHTML, flag, validateTipId);
			      break;
			  case "register-password":  
			      validateHTML.innerHTML = (flag != undefined) ? "" : validatePs(id);
				  registerTip(validateHTML.innerHTML, flag, validateTipId);
			      break;
			  case "register-password-again": 
			      validateHTML.innerHTML = (flag != undefined) ? "" : validatePsAg("register-password", id);
				  registerTip(validateHTML.innerHTML, flag, validateTipId);
			      break;
		      default: 
			      break;
	     }
	}
	
	//验证邮箱
	function validateMail(id){
	    var validateMsg = "";
	    if (id.indexOf("mail") > 1){
	       var mailValue = Hub.dom.getById(id).value;
		   if (regTest.mail.test(mailValue)){
		      if (id.indexOf("register") != -1 || id.indexOf("new-mail") != -1){
				  try{
					var result = window.external.UserMailCheck(mailValue),	//c接口，判断用户名是否用过，以及邮箱是否有效
					    checkResult = result.replace(/^\s+([0-9]{1,3})[0-9a-zA-Z\<\>]{0,}$/, "$1");
					if (checkResult == "45"){
					   validateMsg = "邮箱域名无效";
					}else if (checkResult == "43"){
					   validateMsg = "此用户名已存在";
					}else if (checkResult == "42"){
					   validateMsg = "邮箱地址不存在";
					}
				  }catch(e){
				    //TODO
				  }
			  }
		   }else{
		      validateMsg = "邮箱格式不正确";
		   }
	    }
	    return validateMsg;
	}
	
	//验证密码
	function validatePs(id){
	    var validateMsg = "",
	        password = Hub.dom.getById(id).value;
	    if (!regTest.password.test(password)){
	        if (password.length < 6){
		       validateMsg = "密码过短，最少6位";
		    }else if (password.length > 20){
		       validateMsg = "密码过长，最多20位";
		    }else{
		       validateMsg = "请输入6-20个数字、英文字母或符号；区分大小写";
		    }
	    }
	    return validateMsg;
	}
	
	function validatePsAg(id, idAg){
	    var validateMsg = "",
	        password = Hub.dom.getById(id).value,
	        passwordAg = Hub.dom.getById(idAg).value;
	    if (regTest.password.test(password)){
			if (password != passwordAg){
				validateMsg = "两次输入的密码不一致";
			}
		}else{
		    validateMsg = "请重新输入密码后，再确认";
		}
		return validateMsg;
	}
	
	function registerTip(innerHTML, flag, validateTipId){
		var validateTip = Hub.dom.getById(validateTipId);
	    if (innerHTML == ""){  
		    if (flag != undefined){ 
			  validateTip.style.display = "none";
		    }else{
			  validateTip.style.display = "inline";
			  validateTip.style.background = "url(images/yes.gif) no-repeat 5px 5px";
			}
		}else{
		    validateTip.style.display = "inline";
			validateTip.style.background = "url(images/error.gif) no-repeat 5px 5px";
		}
	}
	
	function inputMouseover(){
	   this.style.background = "url(" + inputImage.over + ") no-repeat";
	}
	
	function inputMouseout(){
	   this.style.background = "url(" + inputImage.out + ") no-repeat " + inputImage.bgX + "px " + inputImage.bgY + "px";
	}
	
	function iconInputMouseover(){
	   this.style.background = "url(images/iconInput.png) no-repeat 3px 1px";
	}
	
	function iconInputMouseout(){
	   this.style.background = "";
	}
	
    return { /*public method*/	   
	   init: function(dom, bgX, bgY){
	      if (dom["name"].tagName == 'INPUT' && dom["color"] != 'undefined' && dom["over"] != 'undefined' && dom["out"] != 'undefined'){
		       dom["name"].style.color = dom.color;
			   dom["name"].value = dom.deflautValue;
			   inputImage.over = dom.over;
			   inputImage.out = dom.out;
			   inputImage.bgX = bgX;
			   inputImage.bgY = bgY;
		  }
	   },
	   
	   initCreatedPsInput: function(element, parentElement, id, type){
	       if (typeof element == 'object' && typeof parentElement == 'object' && element.tagName == "INPUT"){
		       element.id = id;
			   element.type = type;
		       parentElement.appendChild(element);	   
		   }
	   },
	
	
	   inputEffect: function(element){
	      if (typeof element == 'object'){
		    if (element.tagName == "INPUT"){
	            element.onblur = inputBlur;
	            element.onfocus = inputFocus;
		        Hub.dom.parent(element).onmouseover = inputMouseover;
			    Hub.dom.parent(element).onmouseout = inputMouseout;
		    }
		  }
	   },
	   
	   iconInputEffect: function(element){
	      if (typeof element == "object" && element.tagName == "DIV"){
		        element.onmouseover = iconInputMouseover;
				element.onmouseout = iconInputMouseout;
		  }
	   },
	   
	   inputBlur: function(){
	      return inputBlur;
	   },
	   
	   loginCheck: function(id){
	      var loginCheck = Hub.dom.getById(id);
	      loginCheck.onclick = function(){
            	if (Hub.dom.hasClass(this,"checked")){
				    Hub.dom.removeClass(this, "checked");
				}else{
				    Hub.dom.addClass(this, "checked");
				}	  
		  }
	   },
	   
	   validateMail: function(id){
	      return validateMail(id);
	   },
	   
	   validatePs: function(id){
	      return validatePs(id);
	   },
	   
	   validatePsAg: function(id, idAg){
	      return validatePsAg(id, idAg);
	   }
	}
})();



/*select dom
  <div class = "hub-list-div">
     <div class = "hub-list-option">
	    <div class = "hub-list-name"></div>
	    <div class = "hub-list-img"></div> //list div
	 </div>
  </div>
*/

Hub.selectOption = (function(){

    /*private attribute*/
	var optionAttr = {
	    "className": "hub-list-div",
	    "id": Math.random()	
	}
	
	var optionStyle = {
	    "bgColor": "#D9ECFB",
		"positionOver": "url(images/close.gif) no-repeat 0px 9px",
	    "positionOut": "url(images/close.gif) no-repeat -12px 9px"
	};
	
	
    var optionDiv = ["<div class = 'hub-list-option'><div class = 'hub-list-name'>",
		"", //index 1
	    "</div><div class = 'hub-list-img'></div></div>"
    ];
    
	function createDivList(list){
	     var div = [];
	     if (typeof list == 'object' && list.name != 'undefined'){
			 for (var i in list){
				optionDiv.splice(1, 1, list[i].name);
				div.push(optionDiv.join(""));
			 }
		 }
		 return div.join("");
	}
	
	function optionMouseover(){
        if (this.className.indexOf("option") > 1) {
		    this.style.backgroundColor = optionStyle.bgColor;
			this.lastChild.style.visibility = "visible";
	    }else if (this.className.indexOf("img") > 1){
		    this.style.background = optionStyle.positionOver;
		}
	}
	
	function optionMouseout(){
	    if (this.className.indexOf("option") > 1) {
	        this.style.backgroundColor = "";
			this.lastChild.style.visibility = "hidden";
		}else if (this.className.indexOf("img") > 1){
		    this.style.background = optionStyle.positionOut;
		}
	}
	
	function optionClick(e){
	   var target = e ? e.target : window.event.srcElement;
	   Hub.event.stopBubble(e); //阻止冒泡;
	   if (target.className.indexOf("name") > 1){
			var name = this.firstChild.innerHTML.toString();
			var loginInput = Hub.dom.getById("login-mail");
			loginInput.value = name;
			Hub.dom.getById("login-mail-validate").innerHTML = Hub.effect.validateMail("login-mail");//验证
			loginInput.style.color = "#1E1E1E";
			Hub.dom.parent(this).style.display = "none";
       }else if (target.className.indexOf("img") > 1){
			Hub.dom.parent(this).style.display = "none";
	   }
	}
	
	function addEffect(options){
	    if (typeof options === 'object' && options.length > 1){
		    var current;
		    for(var i in options){
			    current = options[i];   
				current.onmouseover = optionMouseover;
				current.onmouseout = optionMouseout;
				current.onclick = optionClick;
			}
		}
	}
	
	function optionFocus(inputId){
	    var input = Hub.dom.getById(inputId);
	    if (window.ActiveXObject){ //IE
		    var range = input.createTextRange();
			range.moveStart("character", input.value.length);
			range.collapse(true);
			range.select();
		}else{
		    input.focus();
		}
	}
	
	function eventOption(element, type, id, eventParam){
	    function showOption(){
	        Hub.dom.getById(id).style.display = "block";
			optionFocus("login-mail");
			setSBValue(id);
	    }
	
	    function hideOption(){
	        Hub.dom.getById(id).style.display = "none";
	    }
	    
	    if (eventParam === "show"){
	        Hub.event.addEvent(element, type, showOption);
		}else if (eventParam === "hide"){
		    Hub.event.addEvent(element, type, hideOption);
		}
	}
	
	function setSBValue(value){
	    Hub.dom.getById("sb").value = value;
	}
	
	function getSBValue(){
	    return Hub.dom.getById("sb").value;
	}
	
	function addListEffect(options, value){
	    if (typeof options === 'object'){
			options.onmouseover = function(){
				setSBValue("1");
				Hub.dom.parent(Hub.dom.getById("login-mail")).style.background = "url(images/input2.png) no-repeat";
			};
			options.onmouseout = function(){
				setSBValue(value);
				Hub.dom.parent(Hub.dom.getById("login-mail")).style.background = "url(images/input.png) no-repeat 3px 3px";
			};
		}
	}
	/*
	*@params element is input	
	*/
	return{
		initOption: function(list, element){
			if (typeof element == 'object' && element.tagName == "INPUT"){
				var inputDiv = Hub.dom.parent(element);
				var inputOffset = Hub.dom.offset(inputDiv);
				var offsetLeftDiv = parseInt(inputOffset.left + 3);	
				var offsetTopDiv = parseInt(inputOffset.top) + parseInt(inputDiv.offsetHeight);
				
				var listDiv = Hub.dom.createE("div");
				var listOption = createDivList(list);
				
				listDiv.id = parseInt((Math.random() / optionAttr.id) * 10000000);
				listDiv.className = optionAttr.className;
				listDiv.style.left = offsetLeftDiv + "px";
				listDiv.style.top = offsetTopDiv + "px";
				listDiv.style.width = parseInt(inputDiv.offsetWidth - 8) + "px";
				listDiv.innerHTML = listOption;
				document.body.appendChild(listDiv);
				
				addEffect(Hub.dom.getByClass("hub-list-option", "div"));
				addEffect(Hub.dom.getByClass("hub-list-img", "div"));
                addListEffect(Hub.dom.getById(listDiv.id), listDiv.id);
				
                return listDiv.id;				
			}   
		},
		
		eventOption: function(element, type, id, eventParam){
		    eventOption(element, type, id, eventParam)
		},
		
		setSBValue: function(value){
		    setSBValue(value);
		},
		
		getSBValue: function(){
		    return getSBValue();
		}
	}
})();