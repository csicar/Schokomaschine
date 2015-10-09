angular.module('profile', ['LocalStorageModule', 'camera', 'now'])
.config(function (localStorageServiceProvider) {
  localStorageServiceProvider
    .setPrefix('schokomaschine')
    .setStorageType('localStorage');
})
.factory('Profile', function(localStorageService, $http, $q){
	var profiles = [];
	var addr = 'http://192.168.4.1';
	var self = function Profile(obj){
		obj = obj || {}
		obj.steps = obj.steps || [];
		obj.temp = obj.temp || 20;
		obj.name = obj.name || 'neu';
		var index = profiles.push(obj)-1;

		obj.save = function(){
			profiles[index] = this;
			console.log(index, profiles);
			Profile.save();
		}

		obj.remove = function(index){
			obj.steps.splice(index, 1);
			console.log(obj)
			return obj.steps;
		}

		obj.run = function(cb) {
			var pos = 0;
			var steps = this.steps;
			var name = this.name;
			function next(){
				cb('step', pos);
				pos += 1;
				var step =  steps[pos-1];
				if(!step) return true;

				var temp = (step.temp*100);
				if(step.type == 'keep'){
					if(step.end == 'button'){
						return $http.get(addr+'/profile/add/keep/button?temp='+temp).then(next);
					}else{
						return $http.get(addr+'/profile/add/keep/button?temp='+temp+'&time='+step.endTime).then(next);
					}
				}else{
					return $http.get(addr+'/profile/add/goto?temp='+temp).then(next);
				}
			}
			return $http.get(addr+'/profile/clear').then(next).then(function(){
				cb('run');
				$http.get(addr+'/profile/run?name='+name);
				console.log('end!');
			});
		};

		return obj;
	}

	self.list = function(){
		return profiles;
	}

	self.find = function(name){
		return profiles.filter(function(profile){
			return profile.name === name;
		})[0];
	}

	self.gotoIndex = function(index){

	}

	self.save = function(){
		localStorageService.set('profiles', profiles);
	}
	if(localStorageService.get('profiles')) localStorageService.get('profiles').forEach(function(profile) {
		self(profile);
	})
	return self;
})
