angular.module('starter.controllers', ['tc.chartjs', 'mp.datePicker'])

.controller('AppCtrl', function($scope, $ionicModal, $timeout) {

  // With the new view caching in Ionic, Controllers are only called
  // when they are recreated or on app start, instead of every page change.
  // To listen for when this page is active (for example, to refresh data),
  // listen for the $ionicView.enter event:
  //$scope.$on('$ionicView.enter', function(e) {
  //});

  // Form data for the login modal
  $scope.loginData = {};

  // Create the login modal that we will use later
  $ionicModal.fromTemplateUrl('templates/login.html', {
    scope: $scope
  }).then(function(modal) {
    $scope.modal = modal;
  });

  // Triggered in the login modal to close it
  $scope.closeLogin = function() {
    $scope.modal.hide();
  };

  // Open the login modal
  $scope.login = function() {
    $scope.modal.show();
  };

  // Perform the login action when the user submits the login form
  $scope.doLogin = function() {
    console.log('Doing login', $scope.loginData);

    // Simulate a login delay. Remove this and replace with your login
    // code if using a login system
    $timeout(function() {
      $scope.closeLogin();
    }, 1000);
  };
})

.controller('HomeCtrl', function($scope, particleService) {
    var d1 = new Date (),
    d2 = new Date (d1);
    d3 = new Date (d1);
    d2.setMinutes (d1.getMinutes() - 8);
    d3.setMinutes (d1.getMinutes() - 2);
    
    getData();
    
    $scope.doRefresh = function() {
        getData();
        $scope.$broadcast('scroll.refreshComplete');
    };
    
    $scope.salinity = 1.025;
    $scope.phUpdate = d3;
    
    $scope.temperatureData = {
      labels: GetDates(d1, 6),
      datasets: [
        {
          label: 'This Week',
          fillColor: 'rgba(220,220,220,0.2)',
          strokeColor: 'rgba(220,220,220,1)',
          pointColor: 'rgba(220,220,220,1)',
          pointStrokeColor: '#fff',
          pointHighlightFill: '#fff',
          pointHighlightStroke: 'rgba(220,220,220,1)',
          data: [80.1, 80.9, 81.2, 80.2, 81.1, 80.9, 79.8]
        }
      ]
    };
    
    $scope.phData = {
      labels: GetDates(d1, 6),
      datasets: [
        {
          label: 'This Week',
          fillColor: 'rgba(220,220,220,0.2)',
          strokeColor: 'rgba(220,220,220,1)',
          pointColor: 'rgba(220,220,220,1)',
          pointStrokeColor: '#fff',
          pointHighlightFill: '#fff',
          pointHighlightStroke: 'rgba(220,220,220,1)',
          data: [8.1, 7.9, 8.2, 8.2, 8.1, 8.0, 8.2]
        }
      ]
    };
    
    $scope.salinityData = {
      labels: GetDates(d1, 6),
      datasets: [
        {
          label: 'This Week',
          fillColor: 'rgba(220,220,220,0.2)',
          strokeColor: 'rgba(220,220,220,1)',
          pointColor: 'rgba(220,220,220,1)',
          pointStrokeColor: '#fff',
          pointHighlightFill: '#fff',
          pointHighlightStroke: 'rgba(220,220,220,1)',
          data: [1.026, 1.025, 1.025, 1.026, 1.024, 1.025, 1.026]
        }
      ]
    };
    
    $scope.options = getChartOptions();
    
    function getData(){
        particleService.getTemperature()
        .then(function(data){
            $scope.temperature = data.result * .1;
            $scope.temperatureUpdate = data.coreInfo.last_heard;
        }, function (error) {
//            $log.error(error);
            $scope.errors = error;
        });

        particleService.getPh()
            .then(function(data){
                $scope.ph = data.result * .1;
                $scope.phUpdate = data.coreInfo.last_heard;
            }, function (error) {
//                $log.error(error);
                $scope.errors = error;
        });
    }

})

.controller('ChartsCtrl', function($scope, $stateParams, $ionicModal, $log, dataService) {
    var now = new Date ();
    $scope.dates = GetDates(now, 6);
    $scope.options = getChartOptions();
    
    $scope.calciumData = getEmptyChartData();
    $scope.khData = getEmptyChartData();
    $scope.mgData = getEmptyChartData();
    $scope.no3Data = getEmptyChartData();
    
    function getHistoryData() {
        dataService.getHistory(7)
            .then(function(data){
                updateChart(data, $scope.calciumData, 'ca');
                updateChart(data, $scope.khData, 'dKh');
                updateChart(data, $scope.mgData, 'mg');
                updateChart(data, $scope.no3Data, 'no3');
            }, function (error) {
                $log.error(error);
                $scope.errors = error;
            });
    };
    
    $ionicModal.fromTemplateUrl('contact-modal.html', {
        scope: $scope,
        animation: 'slide-in-up'
    }).then(function(modal) {
        $scope.modal = modal
    })  

    $scope.openModal = function() {
        $scope.date = new Date ();
        $scope.modal.show()
    }

    $scope.closeModal = function() {
        $scope.modal.hide();
    };

    $scope.save = function(){
        var parameters = {
            'timestamp': moment(this.date).toISOString(),
            'calcium': this.calcium,
            'alkalinity': this.alkalinity,
            'magnesium': this.magnesium,
            'nitrate': this.nitrate,
            'nitrite': this.nitrite,
            'ammonia': this.ammonia
        };
        dataService.save(parameters)
            .then(function (data) {
                $log.info(data);
                getHistoryData();
                $scope.modal.hide();
            }, function (error) {
                $log.error(error);
                $scope.errors = error;
            });
    }

    
    $ionicModal.fromTemplateUrl('datemodal.html', {
        scope: $scope,
        animation: 'slide-in-up'
    }).then(function(modal) {
        $scope.dateModal = modal
    });
    
    $scope.opendateModal = function() {
      $scope.dateModal.show();
    };
    
    $scope.closedateModal = function() {
        $scope.date = moment(this.date, "MM/DD/YYYY");
        $scope.dateModal.hide();
      //$scope.date = modal;
    };
    
    $scope.$on('$destroy', function() {
        $scope.modal.remove();
        $scope.dateModal.remove();
    });

    getHistoryData();
    
    function getEmptyChartData(){
        return {
          labels: {},
          datasets: [
                {
                  label: 'This Week',
                  fillColor: 'rgba(220,220,220,0.2)',
                  strokeColor: 'rgba(220,220,220,1)',
                  pointColor: 'rgba(220,220,220,1)',
                  pointStrokeColor: '#fff',
                  pointHighlightFill: '#fff',
                  pointHighlightStroke: 'rgba(220,220,220,1)',
                  data: [0,0,0,0,0,0,0]
                }
              ]
            };
    };
    
    function updateChart(data, chartData, parameter){
        var nullsRemoved = data
            .filter(function(log){
                if(log.results[0] && !isNaN(log.results[0][parameter]))
                    return log;
            });
        
        chartData.datasets[0].data = nullsRemoved
            .map(function(log){
                return log.results[0][parameter];
            });
        
        chartData.labels = nullsRemoved
            .map(function(log){
                return moment(log.interval.end).format('MM/DD');
        });
    }
})

.controller('ControlCtrl', function($scope, $stateParams, particleService) {

    $scope.relays = [
        {id: 'r1', name: 'Kessil a350w', state: 'auto', scheduled: true},
        {id: 'r2', name: 'Scrubber Lights', state: 'auto', scheduled: true},
        {id: 'r3', name: 'Heater/Skimmer', state: 'on'},
        {id: 'r4', name: 'ATO', state: 'on'}
    ]
    
    $scope.setRelay = function(cmd, relay){
        particleService.relay(relay.id + ',' + cmd);
    };
})

.controller('LightsCtrl', function($scope, $stateParams, $timeout) {

    $scope.getClass = function(){
        return "evening";
    }
//    $scope.top = 40;
//    $scope.moveSun = function(){
//        $scope.showSun = !$scope.showSun;
//        $timeout(function(){
//            $scope.top = $scope.top > 0 ? $scope.top - 2 : 0;
//        }, 500);
//    };
})

.filter('moment', function () {
  return function (input, momentFn /*, param1, param2, ...param n */) {
    var args = Array.prototype.slice.call(arguments, 2),
        momentObj = moment(input);
    return momentObj[momentFn].apply(momentObj, args);
  };
});

function GetDates(startDate, daysToGoBack) {
    var aryDates = [];
    for (var i = daysToGoBack; i >= 0; i--) {
        var currentDate = new Date();
        currentDate.setDate(startDate.getDate() - i);
        aryDates.push(moment(currentDate).format('ddd'));
    }
    return aryDates;
}
    
function getChartOptions(){
    return {
      // Sets the chart to be responsive
      responsive: true,
      scaleShowLabels: false,
      ///Boolean - Whether grid lines are shown across the chart
      scaleShowGridLines : true,
      //String - Colour of the grid lines
      scaleGridLineColor : "rgba(0,0,0,.05)",
      //Number - Width of the grid lines
      scaleGridLineWidth : 1,
      //Boolean - Whether the line is curved between points
      bezierCurve : false,
      //Boolean - Whether to show a dot for each point
      pointDot : true,
      //Number - Radius of each point dot in pixels
      pointDotRadius : 4,
      //Number - Pixel width of point dot stroke
      pointDotStrokeWidth : 1,
      //Number - amount extra to add to the radius to cater for hit detection outside the drawn point
      pointHitDetectionRadius : 20,
      //Boolean - Whether to show a stroke for datasets
      datasetStroke : true,
      //Number - Pixel width of dataset stroke
      datasetStrokeWidth : 2,
      //Boolean - Whether to fill the dataset with a colour
      datasetFill : true,
      // Function - on animation progress
      onAnimationProgress: function(){},
      // Function - on animation complete
      onAnimationComplete: function(){},
      //String - A legend template
      legendTemplate : '<ul class="tc-chart-js-legend"><% for (var i=0; i<datasets.length; i++){%><li><span style="background-color:<%=datasets[i].strokeColor%>"></span><%if(datasets[i].label){%><%=datasets[i].label%><%}%></li><%}%></ul>'
    };
}