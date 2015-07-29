angular.module('starter.services', [])

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

            $http.post(inputUrl, parameters, { 
                    headers: { 'Phant-Private-Key': keys.private },
                    transformRequest: transform
                })
                .success(function(data) {
                    deferred.resolve(data[0]);
                }).error(function(data, status, headers, config) {
                    deferred.reject('Request failed: ' + status);
                });
            return deferred.promise;
        },
        get: function(parameter){
        },
        getHistory: function(days){
            var deferred = $q.defer();
            $http.get(getUrl + '?gte[date]=2015/07/01')
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
        for(var p in json)
        str.push(encodeURIComponent(p) + "=" + encodeURIComponent(json[p]));
        return str.join("&");
    }
});