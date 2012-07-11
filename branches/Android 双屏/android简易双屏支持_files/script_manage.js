/*
	[UCenter Home] (C) 2007-2008 Comsenz Inc.
	$Id: script_manage.js 13178 2009-08-17 02:36:39Z liguode $
*/

//�������
function wall_add(cid, result) {
	if(result) {
		var obj = $('comment_ul');
		var newli = document.createElement("div");
		var x = new Ajax();
		x.get('do.php?ac=ajax&op=comment', function(s){
			newli.innerHTML = s;
		});
		obj.insertBefore(newli, obj.firstChild);
		if($('comment_message')) {
			$('comment_message').value= '';
		}
		//��ʾ��û���
		showreward();
	}
}

//��ӷ���
function share_add(sid, result) {
	if(result) {
		var obj = $('share_ul');
		var newli = document.createElement("div");
		var x = new Ajax();
		x.get('do.php?ac=ajax&op=share', function(s){
			newli.innerHTML = s;
		});
		obj.insertBefore(newli, obj.firstChild);
		$('share_link').value = 'http://';
		$('share_general').value = '';
		//��ʾ��û���
		showreward();
	}
}
//�������quickcommentform_12
function comment_add(id, result) {
	if(result) {
		var obj = $('comment_ul');
		var newli = document.createElement("div");
		var x = new Ajax();
		
		var tt = '';
		if($('comm_num')){
			var n = parseInt($('comm_num').innerHTML);
			if( n > 0){
				var tt = 'tt';
			}
			var c = n + 1;
			$('comm_num').innerHTML = c + '';
		}
		x.get('do.php?ac=ajax&op=comment&tt=' + tt, function(s){
			newli.innerHTML = s;
		});
		if($('comment_prepend')){
			obj = obj.firstChild;
			while (obj && obj.nodeType != 1){
				obj = obj.nextSibling;
			}
			obj.parentNode.insertBefore(newli, obj);
		} else {
			obj.appendChild(newli);
		}
		if($('comment_message')) {
			$('comment_message').value= '';
		}
		if($('comment_replynum')) {
			var a = parseInt($('comment_replynum').innerHTML);
			var b = a + 1;
			$('comment_replynum').innerHTML = b + '';
		}
		
		//��ʾ��û���
		showreward();
	}
}
//�༭
function comment_edit(id, result) {
	if(result) {
		var ids = explode('_', id);
		var cid = ids[1];
		var obj = $('comment_'+ cid +'_li');
		var x = new Ajax();
		x.get('do.php?ac=ajax&op=comment&cid='+ cid, function(s){
			obj.innerHTML = s;
		});
	}
}
//ɾ��
function comment_delete(id, result) {
	if(result) {
		var ids = explode('_', id);
		var cid = ids[1];
		var obj = $('comment_'+ cid +'_li');
		obj.style.display = "none";
		if($('comment_replynum')) {
			var a = parseInt($('comment_replynum').innerHTML);
			var b = a - 1;
			$('comment_replynum').innerHTML = b + '';
		}
	}
}
function doPrnt(id){
	h('blog_print_'+id+'_menu');
	h('blog_print_' + id);
	h('anl_'+id);
	alert('ת�سɹ���');
}
//ɾ��feed
function feed_delete(id, result) {
	if(result) {
		var ids = explode('_', id);
		var feedid = ids[1];
		var obj = $('feed_'+ feedid +'_li');
		obj.style.display = "none";
	}
}

//ɾ������
function share_delete(id, result) {
	if(result) {
		var ids = explode('_', id);
		var sid = ids[1];
		var obj = $('share_'+ sid +'_li');
		obj.style.display = "none";
	}
}
//ɾ������
function friend_delete(id, result) {
	if(result) {
		var ids = explode('_', id);
		var uid = ids[1];
		var obj = $('friend_'+ uid +'_li');
		if(obj != null) obj.style.display = "none";
		var obj2 = $('friend_tbody_'+uid);
		if(obj2 != null) obj2.style.display = "none";
	}
}
//���ķ���
function friend_changegroup(id, result) {
	if(result) {
		var ids = explode('_', id);
		var uid = ids[1];
		var obj = $('friend_group_'+ uid);
		var x = new Ajax();
		x.get('do.php?ac=ajax&op=getfriendgroup&uid='+uid, function(s){
			obj.innerHTML = s;
		});
	}
}
//���ķ�����
function friend_changegroupname(id, result) {
	if(result) {
		var ids = explode('_', id);
		var group = ids[1];
		var obj = $('friend_groupname_'+ group);
		var x = new Ajax();
		x.get('do.php?ac=ajax&op=getfriendname&group='+group, function(s){
			obj.innerHTML = s;
		});
	}
}
//��ӻ���
function post_add(pid, result) {
	if(result) {
		var obj = $('post_ul');
		var newli = document.createElement("div");
		var x = new Ajax();
		x.get('do.php?ac=ajax&op=post', function(s){
			newli.innerHTML = s;
		});
		obj.appendChild(newli);
		if($('message')) {
			$('message').value= '';
			newnode = $('quickpostimg').rows[0].cloneNode(true);
			tags = newnode.getElementsByTagName('input');
			for(i in tags) {
				if(tags[i].name == 'pics[]') {
					tags[i].value = 'http://';
				}
			}
			var allRows = $('quickpostimg').rows;
			while(allRows.length) {
				$('quickpostimg').removeChild(allRows[0]);
			}
			$('quickpostimg').appendChild(newnode);
		}
		if($('post_replynum')) {
			var a = parseInt($('post_replynum').innerHTML);
			var b = a + 1;
			$('post_replynum').innerHTML = b + '';
		}
		//��ʾ��û���
		showreward();
	}
}
//�༭����
function post_edit(id, result) {
	if(result) {
		var ids = explode('_', id);
		var pid = ids[1];

		var obj = $('post_'+ pid +'_li');
		var x = new Ajax();
		x.get('do.php?ac=ajax&op=post&pid='+ pid, function(s){
			obj.innerHTML = s;
		});
	}
}
//ɾ������
function post_delete(id, result) {
	if(result) {
		var ids = explode('_', id);
		var pid = ids[1];
		
		var obj = $('post_'+ pid +'_li');
		obj.style.display = "none";
		if($('post_replynum')) {
			var a = parseInt($('post_replynum').innerHTML);
			var b = a - 1;
			$('post_replynum').innerHTML = b + '';
		}
	}
}
//���к�
function poke_send(id, result) {
	if(result) {
		var ids = explode('_', id);
		var uid = ids[1];

		if($('poke_'+ uid)) {
			$('poke_'+ uid).style.display = "none";
		}
		//��ʾ��û���
		showreward();
	}
}
//��������
function myfriend_post(id, result) {
	if(result) {
		var ids = explode('_', id);
		var uid = ids[1];
		$('friend_'+uid).innerHTML = '<p>���������Ǻ����ˣ����������������ԣ�<a href="space.php?uid='+uid+'#comment" target="_blank">��TA����</a> ������ <a href="cp.php?ac=poke&op=send&uid='+uid+'" id="a_poke_'+uid+'" onclick="ajaxmenu(event, this.id, 1)">����к�</a></p>';
	}
}
//ɾ����������
function myfriend_ignore(id) {
	var ids = explode('_', id);
	var uid = ids[1];
	$('friend_tbody_'+uid).style.display = "none";
}

//����Ⱥ��
function mtag_join(tagid, result) {
	if(result) {
		location.reload();
	}
}

//ѡ��ͼƬ
function picView(albumid) {
	if(albumid == 'none') {
		$('albumpic_body').innerHTML = '';
	} else {
		ajaxget('do.php?ac=ajax&op=album&id='+albumid+'&ajaxdiv=albumpic_body', 'albumpic_body');
	}
}
//ɾ���ط��ʼ�
function resend_mail(id, result) {
	if(result) {
		var ids = explode('_', id);
		var mid = ids[1];
		var obj = $('sendmail_'+ mid +'_li');
		obj.style.display = "none";
	}
}

//����Ӧ�ò��ɼ�
function userapp_delete(id, result) {
	if(result) {
		var ids = explode('_', id);
		var appid = ids[1];
		$('space_app_'+appid).style.display = "none";
	}
}

//do����
function docomment_get(id, result) {
	if(result) {
		var ids = explode('_', id);
		var doid = ids[1];
		var showid = 'docomment_'+doid;
		var opid = 'do_a_op_'+doid;

		$(showid).style.display = '';
		$(showid).className = 'sub_doing';
		ajaxget('cp.php?ac=doing&op=getcomment&doid='+doid, showid);

		if($(opid)) {
			$(opid).innerHTML = '����';
			$(opid).onclick = function() {
				docomment_colse(doid);
			}
		}
		//��ʾ��û���
		showreward();
	}
}

function docomment_colse(doid) {
	var showid = 'docomment_'+doid;
	var opid = 'do_a_op_'+doid;

	$(showid).style.display = 'none';
	$(showid).style.className = '';

	$(opid).innerHTML = '�ظ�';
	$(opid).onclick = function() {
		docomment_get(showid, 1);
	}
}

function docomment_form(doid, id) {
	var showid = 'docomment_form_'+doid+'_'+id;
	var divid = 'docomment_' + doid;
	ajaxget('cp.php?ac=doing&op=docomment&doid='+doid+'&id='+id, showid);
	if($(divid)) {
		$(divid).style.display = '';
	}
}

function docomment_form_close(doid, id) {
	var showid = 'docomment_form_'+doid+'_'+id;
	$(showid).innerHTML = '';
	h("docomment_" + doid);
	$("do_a_op_"+doid).innerHTML = '�ظ�';
	$("do_a_op_"+doid).onclick = function() {
		docomment_get('docomment_'+doid, 1);
	}
}

//feed����
function feedcomment_get(feedid, result) {
	var showid = 'feedcomment_'+feedid;
	var opid = 'feedcomment_a_op_'+feedid;
	$(showid).style.display = '';
	$(showid).className = 'fcomment';
	
	ajaxget('cp.php?ac=feed&op=getcomment&feedid='+feedid, showid);
	if($(opid) != null) {
		$(opid).innerHTML = '����';
		$(opid).onclick = function() {
			feedcomment_close(feedid);
		}
	}
}

function feedcomment_add(id, result) {
	if(result) {
		var ids = explode('_', id);
		var cid = ids[1];

		var obj = $('comment_ol_'+cid);
		var newli = document.createElement("div");
		var x = new Ajax();
		x.get('do.php?ac=ajax&op=comment', function(s){
			newli.innerHTML = s;
		});
		obj.appendChild(newli);

		$('feedmessage_'+cid).value= '';
		//��ʾ��û���
		showreward();
	}
}

//�ر�����
function feedcomment_close(feedid) {
	var showid = 'feedcomment_'+feedid;
	var opid = 'feedcomment_a_op_'+feedid;

	$(showid).style.display = 'none';
	$(showid).style.className = '';

	$(opid).innerHTML = '����';
	$(opid).onclick = function() {
		feedcomment_get(feedid);
	}
}

//�������
function feed_post_result(feedid, result) {
	if(result) {
		location.reload();
	}
}

//��ʾ���ද̬
function feed_more_show(feedid) {
	var showid = 'feed_more_'+feedid;
	var opid = 'feed_a_more_'+feedid;

	$(showid).style.display = '';
	$(showid).className = 'sub_doing';

	$(opid).innerHTML = '&laquo; �����б�';
	$(opid).onclick = function() {
		feed_more_close(feedid);
	}
}

function feed_more_close(feedid) {
	var showid = 'feed_more_'+feedid;
	var opid = 'feed_a_more_'+feedid;

	$(showid).style.display = 'none';

	$(opid).innerHTML = '&raquo; ���ද̬';
	$(opid).onclick = function() {
		feed_more_show(feedid);
	}
}

//����ͶƱ
function poll_post_result(id, result) {
	if(result) {
		var aObj = $('__'+id).getElementsByTagName("a");
		window.location.href = aObj[0].href;
	}
}

//����֮��
function show_click(id) {
	var ids = id.split('_');
	var idtype = ids[1];
	var id = ids[2];
	var clickid = ids[3];
	ajaxget('cp.php?ac=click&op=show&clickid='+clickid+'&idtype='+idtype+'&id='+id, 'click_div');
	//��ʾ��û���
	showreward();
}

//feed�˵�
function feed_menu(feedid, show) {
	var obj = $('a_feed_menu_'+feedid);
	if(obj) {
		if(show) {
			obj.style.display='block';
		} else {
			obj.style.display='none';
		}
	}
	var obj = $('feedmagic_'+feedid);
	if(obj) {
		if(show) {
			obj.style.display='block';
		} else {
			obj.style.display='none';
		}
	}
}

//��д����
function showbirthday(){
	$('birthday').length=0;
	for(var i=0;i<28;i++){
		$('birthday').options.add(new Option(i+1, i+1));
	}
	if($('birthmonth').value!="2"){
		$('birthday').options.add(new Option(29, 29));
		$('birthday').options.add(new Option(30, 30));
		switch($('birthmonth').value){
			case "1":
			case "3":
			case "5":
			case "7":
			case "8":
			case "10":
			case "12":{
				$('birthday').options.add(new Option(31, 31));
			}
		}
	} else if($('birthyear').value!="") {
		var nbirthyear=$('birthyear').value;
		if(nbirthyear%400==0 || nbirthyear%4==0 && nbirthyear%100!=0) $('birthday').options.add(new Option(29, 29));
	}
}
/**
 * ����Ϳѻ
 * @param String fid: Ҫ�رյĲ�ID
 * @param String oid: Ҫ���뵽�����Ŀ��ID
 * @param String url: Ϳѻ�ļ��ĵ�ַ
 * @param String tid: �л���ǩID
 * @param String from: Ϳѻ��������
 * @return û�з���ֵ
 */
function setDoodle(fid, oid, url, tid, from) {
	if(tid == null) {
		hideMenu();
	} else {
		//��������ǩ�л���
		$(tid).style.display = '';
		$(fid).style.display = 'none';
	}
	var doodleText = '[img]'+url+'[/img]';
	if($(oid) != null) {
		if(from == "editor") {
			insertImage(url);
		} else {
			insertContent(oid, doodleText);
		}
	}
}

function selCommentTab(hid, sid) {
	$(hid).style.display = 'none';
	$(sid).style.display = '';
}


//������˸
function magicColor(elem, t) {
	t = t || 10;//��ೢ��
	elem = (elem && elem.constructor == String) ? $(elem) : elem;
	if(!elem){
		setTimeout(function(){magicColor(elem, t-1);}, 400);//���û�м�����ɣ��Ƴ�
		return;
	}
	if(window.mcHandler == undefined) {
		window.mcHandler = {elements:[]};
		window.mcHandler.colorIndex = 0;
		window.mcHandler.run = function(){
			var color = ["#CC0000","#CC6D00","#CCCC00","#00CC00","#0000CC","#00CCCC","#CC00CC"][(window.mcHandler.colorIndex++) % 7];
			for(var i = 0, L=window.mcHandler.elements.length; i<L; i++)
				window.mcHandler.elements[i].style.color = color;
		}
	}
	window.mcHandler.elements.push(elem);
	if(window.mcHandler.timer == undefined) {
		window.mcHandler.timer = setInterval(window.mcHandler.run, 500);
	}
}

//��˽����
function passwordShow(value) {
	if(value==4) {
		$('span_password').style.display = '';
		$('tb_selectgroup').style.display = 'none';
	} else if(value==2) {
		$('span_password').style.display = 'none';
		$('tb_selectgroup').style.display = '';
	} else {
		$('span_password').style.display = 'none';
		$('tb_selectgroup').style.display = 'none';
	}
}

//��˽�ض�����
function getgroup(gid) {
	if(gid) {
		var x = new Ajax();
		x.get('cp.php?ac=privacy&op=getgroup&gid='+gid, function(s){
			s = s + ' ';
			$('target_names').innerHTML += s;
		});
	}
}

function checkEnum(){
	if(parseInt($('num').value) > 9999){
		$('num').value = 9999;
		alert('�µ��ȶ�ֻ����0~9999֮���һ������');
		return false;
	}
	$('changenumsubmit').click();
}