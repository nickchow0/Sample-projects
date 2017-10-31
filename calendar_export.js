/*
	This file takes calendar info from a student and performs the following:
	1) parse course event info
	2) transform calendar info into json object
	3) requests user for Google calendar authentication 
	4) export to the user's calendar
*/
function create_calendar_json() {
	// initialize the json array
	var course_schedule_json_array = []
	// for each course that has been checked
	for (var i = 0; i < $scope.course_event.length; i++) {
		var course_schedule = $scope.course_event[i];
		// add course only if it is checked
		if (course_schedule["checked"]) {
			// getting start and end dates/times for the course
			var course_start_date = new Date(course_schedule["startDate"]);       
			var course_end_date = new Date(course_schedule["endDate"]);
			var course_start_time = course_schedule.start; 
			var course_end_time = course_schedule.end;
			// building string for the recurrence rule
			var days_of_week = eval($scope.course_event[i]["dow"]);
			/* 
				Advance start date of the course to actual first day of class
				i.e. if quarter starts on Mon and this class is every Wed, Fri, 
				then move the start date to the first Wed
			*/ 
			while (course_start_date.getDay() !== days_of_week[0]) {
				course_start_date.setDate(course_start_date.getDate() + 1);
			};
			var day_codes = ['MO','TU','WE','TH','FR','SA','SU'];
			var recurrence_code = 'RRULE:FREQ=WEEKLY;BYDAY=';
			// enumerate through the days for this course
			for (var j = 0; j < days_of_week.length; j++) {
				var day = days_of_week[j] - 1;
				days_of_week[j] = day_codes[day];
			};
			// start and end date and time for the first session          
			var start_date_time = course_start_date.toISOString().slice(0,-13) + course_start_time;
			var end_date_time = course_start_date.toISOString().slice(0,-13) + course_end_time;
			var end_course_date = course_end_date.toISOString().slice(0,-14);
			end_course_date = end_course_date.replace(/-/g,'');
			recurrence_code = recurrence_code.concat(days_of_week.join(','), ';UNTIL=', parseInt(end_course_date) + 1);
			// create a json object to store the necessary data to update calendar
			var course_info_json = {
				'summary' : course_schedule["full_title"],
				'location' : course_schedule["location"],
				'start' : {
					'dateTime' : start_date_time,
					'timeZone' : 'America/Los_Angeles'
				},
				'end' : {
					'dateTime' : end_date_time,
					'timeZone' : 'America/Los_Angeles'
				},
				'recurrence' : [
					recurrence_code
				]
			};
			course_schedule_json_array.push(course_info_json);
		};
	};
return course_schedule_json_array;
};

// Google cal export API
$scope.gcal_export = function() {
	// Array of API discovery doc URLs for APIs used
	var DISCOVERY_DOCS = ["https://www.googleapis.com/discovery/v1/apis/calendar/v3/rest"];
	// Authorization scopes required by the API; multiple scopes can be included, separated by spaces.
	var SCOPES = "https://www.googleapis.com/auth/calendar";

	// On load, called to load the auth2 library and API client library.
	function handleClientLoad() {
		gapi.load('client:auth2', initClient);
	};

	// Initializes the API client library and sets up sign-in state listeners.
	function initClient() {
		gapi.client.init({
			discoveryDocs: DISCOVERY_DOCS,
			clientId: CLIENT_ID,
			scope: SCOPES
		}).then(function (response) {
			// Listen for sign-in state changes.
			gapi.auth2.getAuthInstance().isSignedIn.listen(updateSigninStatus);
			// Handle the initial sign-in state.
			updateSigninStatus(gapi.auth2.getAuthInstance().isSignedIn.get());
		});
	};

	// Called when the signed in status changes
	function updateSigninStatus(isSignedIn) {
		// check if user is already signed in
		if (isSignedIn) {
			update_calendar();
		} else {
			// request user to sign in
			gapi.auth2.getAuthInstance().signIn();
		};
	};

	// add each course to calendar
	function add_events_to_calendar(calendar_id) {
		// calls the function to create an array of json objects for each course
		var course_schedule_json_array = create_calendar_json();
		// loop through the course schedule array and add each course occurance to calendar
		for (var i = 0; i < course_schedule_json_array.length; i++) {
			var request = gapi.client.calendar.events.insert({
				'calendarId' : calendar_id,
				'resource' : course_schedule_json_array[i]
			});
			request.execute(function(event) {
				// print link to the calendar event
				// console.log(event.htmlLink);
				// show successful message
				$('#calendar_export_success').show();
				setTimeout(function(){
					$("#calendar_export_success").fadeOut(500);
				}, 3000);        
			});
		};  
	};

	function update_calendar() {
		// modal pop-up to show disclaimer
		$('#calendar-modal').modal('toggle');
		// if user agrees to proceed
		$scope.calendar_disclaimer_acceptance = function() {
			var added_to_calendar = 0;

			function check_user_calendars(calendars) {
				// if Stanford Carta calendar already exists, add it to that calendar
				for (var i = 0; i < calendars.length; i++) {
					if (calendars[i]['summary'] === 'Stanford Carta') {
						var calendar_id = calendars[i]['id'];
						add_events_to_calendar(calendar_id);
						added_to_calendar = 1;
					}; 
				};
				// if the calendar doesn't exist, create a new calendar called 'Stanford Carta'
				if (added_to_calendar === 0) {
					var request1 = gapi.client.calendar.calendars.insert({
						"resource" : {
							"summary": "Stanford Carta",
							"timezone" : "America/Los_Angeles"
						}
					});
					request1.execute(function(calendar) {
						var calendar_id = calendar['id']
						add_events_to_calendar(calendar_id)
					});
				};
			};

			// get an json object for each calendar from the user's account
			function get_calendars(request, callback) {
				// executes the api request
				request.execute(function(resp){
					var calendars = resp.items;
					callback(calendars);
				}); 
			};

			// request to get the list of calendars
			var request = gapi.client.calendar.calendarList.list();
			get_calendars(request, check_user_calendars);
		};
	};
	handleClientLoad();
};