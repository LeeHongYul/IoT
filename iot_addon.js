function XHR_write(data){
        var xhr = new XMLHttpRequest();

        xhr.open("GET","remote_con.php?" + data);
        xhr.send();
}

function XHR_read(data){
        var xhr = new XMLHttpRequest();

        xhr.open("GET","remote_con.php?" + data, false);
        xhr.send();
		
	return xhr.responseText;
}



//--------------------------------------------------------//

function REDON(){
	XHR_write('REDON');
	
	document.RED.src='img/RGB_r.png';
}

function REDOFF(){
	XHR_write('REDOFF');
	
	document.RED.src='img/RGB_off.png';
}

function GREENON(){
	XHR_write('GREENON');
	
	document.GREEN.src='img/RGB_g.png';
}

function GREENOFF(){
	XHR_write('GREENOFF');
	
	document.GREEN.src='img/RGB_off.png';
}

function BLUEON(){
	XHR_write('BLUEON');
	
	document.BLUE.src='img/RGB_b.png';
}

function BLUEOFF(){
	XHR_write('BLUEOFF');

	document.BLUE.src='img/RGB_off.png';
}



function LIMIT(){
	var val=XHR_read('LIMIT');

	if(val==1){
XHR_write('BUZZERON');		
		document.LIMIT.src='img/limit_on.png';
	}else{
XHR_write('BUZZEROFF');
		document.LIMIT.src='img/limit_off.png';
	}
}

function LIMITON(){
XHR_write('BUZZERON');
		document.LIMIT.src='img/limit_on.png';
}
function LIMITOFF(){
XHR_write('BUZZEROFF');
		document.LIMIT.src='img/limit_off.png';
}


	







