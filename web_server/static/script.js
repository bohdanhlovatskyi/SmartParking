var y_data = 0;
var time = "";

var ctx = $("#chart");

var magnetic_sensing_chart = new Chart(ctx, {
    type: 'line',
    data: {
        labels: [],
        datasets : [
            {
                label: 'magnetic field induction force',
                data: [y_data],
                borderColor: [
                    '#060666',
                ],
                borderWidth: 3,
                fill: false,
                yAxisID: "magnetic"
            }
        ]
    },
    
    options: {
        responsive: false,
        scales:{
            xAxes: [ {
                //type: 'time',
                display: true,
                scaleLabel : {
                    display: true,
                    labelString: 'Time (s)'
                    
                    },
                ticks: {
                   autoSkip: true,
                   maxTicksLimit: 12
                }
                }],
            yAxes: [ {
                id: "magnetic",
                display: true,
                position: 'left',
                ticks: {
                    suggestedMin: -15,
                    suggestedMax: 15
                    },
                scaleLabel : {
                    display: true,
                    labelString: 'Magnetic force (nT)'

                    }
                }]
            }

    }

});


setInterval(function(){
    var getData = $.get('/update');
    getData.done(function(results){
        if (magnetic_sensing_chart.data.datasets[0].data.length > 20) { 
            magnetic_sensing_chart.data.labels.shift();
            magnetic_sensing_chart.data.datasets[0].data.shift();
        }
        magnetic = results.results[0];
        time = results.results[1];
        console.log(magnetic);
        console.log(time);
        magnetic_sensing_chart.data.labels.push(time);
        magnetic_sensing_chart.data.datasets[0].data.push(magnetic);
    });
    
    magnetic_sensing_chart.update();
},1000);
