angular.module('now').controller('NowController', function NowController($scope, $timeout, $http, nowCom) {
	var chartElem = document.getElementById('chart');
	var chartLength = 0;

	var data = {
		labels: [],
		datasets: [
			{
				label: "Temperatur",
				fillColor: "rgba(151,187,205,0.2)",
				strokeColor: "rgba(151,187,205,1)",
				pointColor: "rgba(151,187,205,1)",
				pointStrokeColor: "#fff",
				pointHighlightFill: "#fff",
				pointHighlightStroke: "rgba(220,220,220,1)",
				data: []
			},
			{
				label: "Eingeschaltet",
				fillColor: "rgba(220,220,220,0.2)",
				strokeColor: "rgba(220,220,220,1)",
				pointColor: "rgba(220,220,220,1)",
				pointStrokeColor: "#fff",
				pointHighlightFill: "#fff",
				pointHighlightStroke: "rgba(151,187,205,1)",
				data: [],
			}
		]
	};			
	var chart = new Chart(chartElem.getContext("2d")).Line(data, {
		scaleOverride : true,
	    scaleSteps : 8,
	    scaleStepWidth : 5,
	    scaleStartValue : 15,
	    responsive : true, 
	});
	nowCom.on('data', function(data){
		var date = new Date();
		var temp = data.currTemp;
		console.log(temp)
		if(chartLength > 7) {chart.removeData();}
		chart.addData([temp, 0], date.getHours()+':'+date.getMinutes()+'\''+date.getSeconds());
		
		chartLength += 1;
	});
  	
});