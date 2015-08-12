angular.module('starter.services', [])

.factory('particleService', function($http, $q){
    var keys = JSON.parse(window.localStorage['sparkKeys']);
    var getUrl = 'https://api.particle.io/v1/devices/' + keys.deviceId;
    return {
        getTemperature: function(){
            var deferred = $q.defer();
            var now = new Date();
            $http.get(getUrl + '/temperature?access_token=' + keys.accessToken)
            .success(function (data) {   
                deferred.resolve(data);
            }).error(function (data, status, headers, config) {
                deferred.reject('Request failed: ' + status);
            });
            return deferred.promise;
        },
        getPh: function(){
            var deferred = $q.defer();
            var now = new Date();
            $http.get(getUrl + '/ph?access_token=' + keys.accessToken)
            .success(function (data) {   
                deferred.resolve(data);
            }).error(function (data, status, headers, config) {
                deferred.reject('Request failed: ' + status);
            });
            return deferred.promise;
        },
        
        relay: function(command){
            alert(command);
//            var deferred = $q.defer();
//            var now = new Date();
//            $http.get(getUrl + '/relay?access_token=' + keys.accessToken)
//            .success(function (data) {   
//                deferred.resolve(data);
//            }).error(function (data, status, headers, config) {
//                deferred.reject('Request failed: ' + status);
//            });
//            return deferred.promise;
        }
        
    };
})
.factory('dataService', function($http, $q) {
    var keys = JSON.parse(window.localStorage['sparkKeys']);
    var inputUrl = 'https://data.sparkfun.com/input/' + keys.public;
    var getUrl = 'https://data.sparkfun.com/output/' + keys.public + '.json';
    
    return {
        all: function(){
        },
        save: function(parameters){
            var deferred = $q.defer();
            //var xsrf = JSON.stringify(parameters);
            var values = transform(parameters);
            var url = inputUrl + '?callback=JSON_CALLBACK&private_key=' + keys.private + '&' + values;
            $http.jsonp(url)
                .success(function(data) {
                    if(data.success){
                        deferred.resolve(data)
                    }
                    else{
                        deferred.reject('Request failed: ' + data.message);
                    }
                }).error(function(data, status, headers, config) {
                    deferred.reject('Request failed: ' + status);
                });
            return deferred.promise;
        },
        get: function(parameter){
        },
        getHistory: function(days){
            var deferred = $q.defer();
            var now = new Date();
            var searchDate = now.setMinutes(now.getMinutes() - days * 60 * 24);
            $http.get(getUrl + '?gte[date]=' + searchDate)
            .success(function (data) {   
                deferred.resolve(data);
            }).error(function (data, status, headers, config) {
                deferred.reject('Request failed: ' + status);
            });
            return deferred.promise;
        }
    };
    
    function transform(json) {
        var str = [];
        for(var p in json){
            var v = json[p] ? encodeURIComponent(json[p]) : null;
            str.push(encodeURIComponent(p) + "=" + v);
        }
        return str.join("&");
    }
});