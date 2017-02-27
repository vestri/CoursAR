document.addEventListener('DOMContentLoaded', function(){

	var accelElement = document.querySelector('#accel');
	var gyroElement = document.querySelector('#gyro');

	var canvas = document.querySelector('#theCanvas');
	var context = canvas.getContext("2d");

	var currentX = 0;
	var stepX = 2;
	var positionY = canvas.height / 2;
	var mult = -3;

	function draw(value, color){
		
		if (currentX > canvas.width) {
			currentX = 0;
			context.clearRect(0, 0, canvas.width, canvas.height);
		}

		context.beginPath();
		context.fillStyle=color;
		context.rect(currentX, positionY, stepX, value* mult);
		context.fill();
		context.closePath();

		currentX += stepX + 1;
	}

	function handleAccel(event){
		accelElement.innerHTML  = "";

		if (event.acceleration) {
			accelElement.innerHTML += "x: " + event.acceleration.x;
			accelElement.innerHTML += "<br/>y: " + event.acceleration.y;
			accelElement.innerHTML += "<br/>z: " + event.acceleration.z;

		} else {
			accelElement.innerHTML += "No acceleration.";
		}

		if (event.accelerationIncludingGravity) {
			accelElement.innerHTML += "<br/>gravity x : " + event.accelerationIncludingGravity.x;
			accelElement.innerHTML += "<br/>gravity y: " + event.accelerationIncludingGravity.y;
			accelElement.innerHTML += "<br/>gravity z: " + event.accelerationIncludingGravity.z;

			draw(event.accelerationIncludingGravity.x, "red");
			draw(event.accelerationIncludingGravity.y, "green");
			draw(event.accelerationIncludingGravity.z, "blue");

		} else {
			accelElement.innerHTML += "<br/>No accelerationIncludingGravity.";
		}

	}

	function handleOrient(event){	
		gyroElement.innerHTML = "compass: " + (event.webkitCompassHeading ? event.webkitCompassHeading : "null");
		gyroElement.innerHTML += "<br/>alpha: " + event.alpha;
		gyroElement.innerHTML += "<br/>beta: " + event.beta;
		gyroElement.innerHTML += "<br/>gamma: " + event.gamma;
	}
	
	gyroElement.innerHTML = "Orientation : not supported."
	accelElement.innerHTML = "Motion : not supported."
	
	window.addEventListener('devicemotion', handleAccel);
	window.addEventListener('deviceorientation', handleOrient);
	
});