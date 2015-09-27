angular.module('now', ['LocalStorageModule', 'camera'])
.factory('nowCom', function($http, $timeout) {
  var addr = 'http://192.168.1.183'
	var data = {commands: []};
	var cbs = {};
  var lastContact;
  var statusTable = {
    '0': 'off',
    '1': 'firstTime',
    '2': 'started',
    '3': 'finished',
  }
	function callCBs(evt, data){
		(cbs[evt] || []).forEach(function(cb){
			cb(data);
		});
	}
	function getData(){
		$http.get(addr+'/profile', {timeout: 10000}).then(function(res){
			data = parseData(res.data);
			callCBs('data', data);
			$timeout(getData, 500);
		}).catch(function(){
			$timeout(getData, 500);
      callCBs('timeout');
		});
  	}
  	function parseData(data){
      lastContact = new Date();
  		data.currTemp = data.currTemp / 100;

  		var commands = data.commands;
  		commands = commands.split(';');
  		commands = commands.map(function(command){
  			return command.split(',');
  		}).map(function(command){
  			command.pop();
        console.log(4, command[4], statusTable[command[4]]);
  			return {
          type: command[0]?'keep':'goto',
          temperature: (+command[1])/100,
          until: command[2]?'button':'time',
          endTime: (+command[3]),
          status: statusTable[command[4]],
        };
  		})
  		data.commands = commands;
  		console.log(data);
  		return data;
  	}
  	getData();


  	var self = {
  		on: function(evt, cb){
  			cbs[evt] = cbs[evt] || [];
  			cbs[evt].push(cb);
  		},
  		getData: function(){
  			return data;
  		},
      gotoStep: function(index){
        $http.get(addr+'/profile/gotoStep?step='+index);
      }
  	};

    self.on('timeout', function(){
      
    }); 

    return self;
})
