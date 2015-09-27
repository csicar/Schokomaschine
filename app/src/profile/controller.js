angular.module('profile').controller('ProfileController', 
	['$scope', 'Profile', '$routeParams', '$mdToast', 'nowCom', function($scope, Profile, $routeParams, $mdToast, nowCom) {
	if($routeParams.profileName){
		$scope.profile = Profile.find($routeParams.profileName);
		console.log($routeParams.profileName);
	}else{
		$scope.profile = new Profile();
	}
	$scope.data = nowCom.getData();
	$scope.profiles = Profile.list() || [];
	$scope.create = function(type){
		$scope.profile.steps.push({
			type: type,
		})
	}

	$scope.remove = function(index){
		$scope.profile.remove(index);
		$scope.profiles = Profile.list();
	}

	$scope.isActive = function(index){
		if(($scope.data.profileName === $scope.profile.name) &&
			$scope.data.commands[index]) {
			return ($scope.data.commands[index].status === 'started');
		}else{
			return false;
		}
	}

	$scope.save = function(){
		$mdToast.show($mdToast.simple()
	        .content('Wird gespeichert..')
	        .hideDelay(1000));
		console.log($scope.profile.name);
		$scope.profile.save();
		$mdToast.show($mdToast.simple()
	        .content('Profile Saved!')
	        .hideDelay(3000));
	}
	$scope.gotoStep = function(index){
		$mdToast.show($mdToast.simple()
	        .content('Gehe zu Step...')
	        .hideDelay(1000));
		nowCom.gotoStep(index);
		$mdToast.show($mdToast.simple()
	        .content('Zu step gesprungen!')
	        .hideDelay(3000));
	}
	nowCom.on('data', function(data){
		$scope.data = data;
	});
	$scope.run = function(){
		$mdToast.show($mdToast.simple()
	        .content('Uploading profile...')
	        .hideDelay(3000));
		$scope.profile.run(function(evt, data){
			if(evt == 'step'){
				$mdToast.show($mdToast.simple()
			        .content('Uploaded step '+data)
			        .hideDelay(2000));
			}else if(evt == 'run'){
				$mdToast.show($mdToast.simple()
			        .content('Running profile...')
			        .hideDelay(2000));
			}
		}).then(function(){
			$mdToast.show($mdToast.simple()
	        .content('Profile uploaded and started!')
	        .hideDelay(3000));
		});
	}
}]);