#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <time.h>

// ========== USER CONFIGURATION SECTION ==========
// Customize these values for your own setup

// WiFi networks - add as many as you need
const char* wifiCredentials[][2] = {
  {"YourWiFiSSID1", "YourPassword1"},
  {"YourWiFiSSID2", "YourPassword2"},
  // Add more networks as needed
};
const int numNetworks = sizeof(wifiCredentials) / sizeof(wifiCredentials[0]);

// Face Care Routines - customize these for your skincare routine
const struct {
  const char* title;
  const char* subtitle;
  const char* tasks[10]; // Max 10 tasks per routine
  const char* tip;
} morningFaceRoutine = {
  "🌅 Morning Routine",
  "Start your day with radiant skin",
  {
    "Cleanser",
    "Toner",
    "Vitamin C serum",
    "Moisturizer",
    "Sunscreen",
    "" // Empty string marks end of tasks
  },
  "Apply sunscreen as the last step of your morning routine"
};

const struct {
  const char* weekday[7]; // 0=Sunday, 1=Monday,...6=Saturday
  const char* subtitle[7];
  const char* tasks[7][10]; // 7 days, max 10 tasks each
  const char* tip[7];
} nightFaceRoutines = {
  { // Titles
    "🌙 Sunday Night Reset",
    "🌙 Monday Night Resurface",
    "🌙 Tuesday Night",
    "🌙 Wednesday Night Balance",
    "🌙 Thursday Night Resurface",
    "🌙 Friday Night Renewal",
    "🌙 Saturday Night Recovery"
  },
  { // Subtitles
    "Skin rest day",
    "Glycolic acid night",
    "Retinol night",
    "Hydration and soothing night",
    "Glycolic acid Night",
    "Second retinol night this week",
    "Gentle hydration focus"
  },
  { // Tasks for each day
    {"Cleanser", "Moisturizer", ""}, // Sunday
    {"Cleanser", "Glycolic acid", "Moisturizer", ""}, // Monday
    {"Cleanser", "Retinol Serum", "Moisturizer", ""}, // Tuesday
    {"Cleanser", "Hyaluronic acid", "Moisturizer", ""}, // Wednesday
    {"Cleanser", "Glycolic acid", "Moisturizer", ""}, // Thursday
    {"Cleanser", "Retinol treatment", "Moisturizer", ""}, // Friday
    {"Cleanser", "Hyaluronic acid", "Moisturizer", ""}  // Saturday
  },
  { // Tips for each day
    "Give your skin a break from actives once a week",
    "Don't forget to wait between glycolic acid and moisturizer",
    "Apply retinol to dry skin to reduce irritation",
    "Hyaluronic acid works best on damp skin",
    "Glycolic acid makes skin sun-sensitive - always wear SPF next day",
    "Consider a thicker moisturizer after retinol",
    "Weekends are great for extended masking"
  }
};

// Hand Care Routines - customize these for your hand care routine
const struct {
  const char* title;
  const char* subtitle;
  const char* tasks[10];
  const char* tip;
} morningHandRoutine = {
  "✋ Morning Hand Care",
  "Protect your hands for the day",
  {
    "Hand wash",
    "Hand cream",
    "SPF on hands",
    "" // Empty string marks end of tasks
  },
  "Reapply hand SPF every 2-3 hours when outdoors"
};

const struct {
  const char* weekday[7]; // 0=Sunday, 1=Monday,...6=Saturday
  const char* subtitle[7];
  const char* tasks[7][10]; // 7 days, max 10 tasks each
  const char* tip[7];
} nightHandRoutines = {
  { // Titles
    "✋ Sunday Hand Care",
    "✋ Monday Hand Care",
    "✋ Tuesday Hand Care",
    "✋ Wednesday Hand Care",
    "✋ Thursday Hand Treatment",
    "✋ Friday Hand Care",
    "✋ Saturday Hand Treatment"
  },
  { // Subtitles
    "Recovery night",
    "Glycolic acid night for hands",
    "Basic night",
    "Hydration focus",
    "Second glycolic night",
    "Prep for the weekend",
    "Third glycolic night"
  },
  { // Tasks for each day
    {"Wash hands", "Hand cream", ""}, // Sunday
    {"Wash hands", "Glycolic acid", "Hand cream", ""}, // Monday
    {"Wash hands", "Hand cream", ""}, // Tuesday
    {"Wash hands", "Hand cream", ""}, // Wednesday
    {"Wash hands", "Glycolic acid", "Hand cream", ""}, // Thursday
    {"Wash hands", "Hand cream", ""}, // Friday
    {"Wash hands", "Glycolic acid", "Hand cream", ""}  // Saturday
  },
  { // Tips for each day
    "Sunday is perfect for nail and cuticle care",
    "Glycolic acid helps with dark spots on hands",
    "Niacinamide helps with skin barrier",
    "Hydrated hands heal faster",
    "Focus on knuckles and nail areas",
    "Massage improves product absorption",
    "Great night for overnight glove treatment"
  }
};

// ========== END OF USER CONFIGURATION ==========

AsyncWebServer server(80);

void connectToWiFi() {
  Serial.println("Attempting to connect to WiFi...");
  
  for (int i = 0; i < numNetworks; i++) {
    const char* ssid = wifiCredentials[i][0];
    const char* password = wifiCredentials[i][1];
    
    Serial.printf("Trying network %d: %s\n", i+1, ssid);
    WiFi.begin(ssid, password);
    
    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 15) {
      delay(1000);
      Serial.print(".");
      retries++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.printf("\nConnected to %s\n", ssid);
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      return;
    }
    
    Serial.printf("\nFailed to connect to %s\n", ssid);
    WiFi.disconnect();
    delay(1000);
  }
  
  Serial.println("Failed to connect to any network. Restarting...");
  ESP.restart();
}

String generateJSRoutineObject(const String& varName, const String& title, const String& subtitle, 
                             const String tasks[], const String& tip) {
  String js = "const " + varName + " = {\n";
  js += "  title: \"" + title + "\",\n";
  js += "  subtitle: \"" + subtitle + "\",\n";
  js += "  tasks: [\n";
  
  for (int i = 0; tasks[i] != "" && i < 10; i++) {
    js += "    \"" + tasks[i] + "\",\n";
  }
  
  js += "  ],\n";
  js += "  tip: \"" + tip + "\"\n";
  js += "};\n";
  
  return js;
}

String generateJSDayRoutineObject(const String& varName, const String titles[], const String subtitles[], 
                                const String tasks[][10], const String tips[]) {
  String js = "const " + varName + " = {\n";
  
  // Add days
  for (int day = 0; day < 7; day++) {
    String dayName;
    switch(day) {
      case 0: dayName = "Sunday"; break;
      case 1: dayName = "Monday"; break;
      case 2: dayName = "Tuesday"; break;
      case 3: dayName = "Wednesday"; break;
      case 4: dayName = "Thursday"; break;
      case 5: dayName = "Friday"; break;
      case 6: dayName = "Saturday"; break;
    }
    
    js += "  \"" + dayName + "\": {\n";
    js += "    title: \"" + titles[day] + "\",\n";
    js += "    subtitle: \"" + subtitles[day] + "\",\n";
    js += "    tasks: [\n";
    
    for (int i = 0; tasks[day][i] != "" && i < 10; i++) {
      js += "      \"" + tasks[day][i] + "\",\n";
    }
    
    js += "    ],\n";
    js += "    tip: \"" + tips[day] + "\"\n";
    js += "  }";
    
    if (day < 6) js += ",";
    js += "\n";
  }
  
  js += "};\n";
  return js;
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  connectToWiFi();

  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("Waiting for NTP time sync...");
  delay(2000);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = R"rawliteral(
<!DOCTYPE html>
<html lang='en'>
<head>
  <meta charset='UTF-8'>
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>
  <title>Glow Guide ✨</title>
  <link href="https://fonts.googleapis.com/css2?family=Poppins:wght@300;400;500;600&family=Playfair+Display:wght@400;500&display=swap" rel="stylesheet">
  <style>
    :root {
      /* Light Mode Colors */
      --primary-light: #FF9AA2;
      --secondary-light: #FFB7B2;
      --accent-light: #FFDAC1;
      --bg-light: #f8f9fa;
      --card-light: #ffffff;
      --text-light: #333333;
      --border-light: rgba(0,0,0,0.1);
      
      /* Dark Mode Colors */
      --primary-dark: #FF7B8B;
      --secondary-dark: #D8A7B1;
      --accent-dark: #B5EAD7;
      --bg-dark: #121212;
      --card-dark: #1E1E1E;
      --text-dark: #E0E0E0;
      --border-dark: rgba(255,255,255,0.1);
      
      /* Current Mode Defaults */
      --primary: var(--primary-light);
      --secondary: var(--secondary-light);
      --accent: var(--accent-light);
      --bg: var(--bg-light);
      --card: var(--card-light);
      --text: var(--text-light);
      --border: var(--border-light);
    }
    
    .dark-mode {
      --primary: var(--primary-dark);
      --secondary: var(--secondary-dark);
      --accent: var(--accent-dark);
      --bg: var(--bg-dark);
      --card: var(--card-dark);
      --text: var(--text-dark);
      --border: var(--border-dark);
    }
    
    * {
      box-sizing: border-box;
      margin: 0;
      padding: 0;
      transition: background 0.3s, color 0.1s;
    }
    
    body {
      font-family: 'Poppins', sans-serif;
      background: var(--bg);
      color: var(--text);
      min-height: 100vh;
      padding: 2rem;
      display: flex;
      flex-direction: column;
      align-items: center;
    }
    
    .container {
      width: 100%;
      max-width: 600px;
      margin: 0 auto;
    }
    
    header {
      text-align: center;
      margin-bottom: 2rem;
      width: 100%;
      position: relative;
    }
    
    h1 {
      font-family: 'Playfair Display', serif;
      font-weight: 500;
      font-size: 2.4rem;
      margin-bottom: 0.5rem;
      background: linear-gradient(90deg, var(--primary), var(--secondary));
      -webkit-background-clip: text;
      -webkit-text-fill-color: transparent;
    }
    
    .subtitle {
      font-size: 1rem;
      opacity: 0.8;
      margin-bottom: 1rem;
    }
    
    .theme-toggle {
      position: absolute;
      top: 0;
      right: 0;
      background: var(--card);
      border: none;
      width: 40px;
      height: 40px;
      border-radius: 50%;
      display: flex;
      align-items: center;
      justify-content: center;
      cursor: pointer;
      box-shadow: 0 2px 10px rgba(0,0,0,0.1);
    }
    
    .dark-mode .theme-toggle {
      box-shadow: 0 2px 10px rgba(0,0,0,0.3);
    }
    
    .day-display {
      background: var(--card);
      border-radius: 50px;
      padding: 0.6rem 1.2rem;
      display: inline-block;
      font-weight: 500;
      box-shadow: 0 2px 10px rgba(0,0,0,0.05);
      margin-bottom: 1rem;
      font-size: 0.9rem;
    }
    
    .dark-mode .day-display {
      box-shadow: 0 2px 10px rgba(0,0,0,0.2);
    }
    
    .routine-card {
      background: var(--card);
      border-radius: 18px;
      padding: 1.8rem;
      margin-bottom: 2rem;
      box-shadow: 0 8px 25px rgba(0,0,0,0.08);
      border: 1px solid var(--border);
    }
    
    .dark-mode .routine-card {
      box-shadow: 0 8px 25px rgba(0,0,0,0.2);
    }
    
    .routine-header {
      display: flex;
      align-items: center;
      margin-bottom: 1.5rem;
    }
    
    .routine-icon {
      width: 48px;
      height: 48px;
      border-radius: 50%;
      display: flex;
      align-items: center;
      justify-content: center;
      margin-right: 1.2rem;
      font-size: 1.4rem;
      flex-shrink: 0;
    }
    
    .morning-icon {
      background: linear-gradient(135deg, var(--primary), var(--secondary));
      color: white;
    }
    
    .night-icon {
      background: linear-gradient(135deg, #7B8BFF, #A7B1D8);
      color: white;
    }
    
    .hand-icon {
      background: linear-gradient(135deg, #B58BFF, #D8A7B1);
      color: white;
    }
    
    .routine-title {
      font-weight: 600;
      font-size: 1.3rem;
      margin-bottom: 0.3rem;
    }
    
    .routine-subtitle {
      font-size: 0.9rem;
      opacity: 0.7;
    }
    
    .task-list {
      list-style: none;
      margin: 1.2rem 0;
    }
    
    .task-item {
      display: flex;
      align-items: center;
      padding: 1rem 0;
      border-bottom: 1px solid var(--border);
    }
    
    .task-item:last-child {
      border-bottom: none;
    }
    
    .task-checkbox {
      -webkit-appearance: none;
      -moz-appearance: none;
      appearance: none;
      width: 24px;
      height: 24px;
      border: 2px solid var(--border);
      border-radius: 7px;
      margin-right: 1.2rem;
      cursor: pointer;
      position: relative;
      flex-shrink: 0;
    }
    
    .task-checkbox:checked {
      background-color: var(--primary);
      border-color: var(--primary);
    }
    
    .task-checkbox:checked::after {
      content: '✓';
      position: absolute;
      color: white;
      font-size: 1rem;
      top: 50%;
      left: 50%;
      transform: translate(-50%, -50%);
    }
    
    .task-label {
      flex: 1;
      font-size: 1rem;
    }
    
    .task-checkbox:checked + .task-label {
      opacity: 0.6;
      text-decoration: line-through;
    }
    
    .progress-container {
      width: 100%;
      height: 8px;
      background: var(--border);
      border-radius: 4px;
      margin: 1.5rem 0;
      overflow: hidden;
    }
    
    .progress-bar {
      height: 100%;
      background: linear-gradient(90deg, var(--primary), var(--secondary));
      width: 0%;
      transition: width 0.5s ease;
    }
    
    .progress-text {
      font-size: 0.85rem;
      text-align: right;
      opacity: 0.7;
      margin-top: -1rem;
      margin-bottom: 1rem;
    }
    
    .tip-box {
      background: var(--card);
      border-left: 4px solid var(--primary);
      padding: 1rem;
      margin: 1.5rem 0 0;
      border-radius: 0 12px 12px 0;
      font-size: 0.9rem;
      box-shadow: 0 3px 10px rgba(0,0,0,0.03);
    }
    
    .dark-mode .tip-box {
      box-shadow: 0 3px 10px rgba(0,0,0,0.15);
    }
    
    .tip-title {
      font-weight: 600;
      margin-bottom: 0.4rem;
      color: var(--primary);
    }
    
    .completion-message {
      background: var(--card);
      border-radius: 18px;
      padding: 2rem;
      text-align: center;
      margin: 2rem 0;
      box-shadow: 0 8px 25px rgba(0,0,0,0.08);
      border: 1px solid var(--border);
      display: none;
    }
    
    .completion-title {
      font-weight: 600;
      font-size: 1.4rem;
      margin-bottom: 0.8rem;
      background: linear-gradient(90deg, var(--primary), var(--secondary));
      -webkit-background-clip: text;
      -webkit-text-fill-color: transparent;
    }
    
    .completion-subtitle {
      font-size: 1rem;
      opacity: 0.8;
      margin-bottom: 1.2rem;
    }
    
    .time-remaining {
      font-size: 1.1rem;
      font-weight: 500;
      color: var(--primary);
      margin-top: 1rem;
    }
    
    .refresh-btn {
      background: linear-gradient(90deg, var(--primary), var(--secondary));
      color: white;
      border: none;
      padding: 1rem 2rem;
      border-radius: 50px;
      font-size: 1rem;
      font-weight: 500;
      cursor: pointer;
      margin-top: 1.5rem;
      box-shadow: 0 5px 20px rgba(255, 154, 162, 0.3);
      transition: transform 0.3s, box-shadow 0.3s;
    }
    
    .refresh-btn:hover {
      transform: translateY(-2px);
      box-shadow: 0 7px 25px rgba(255, 154, 162, 0.4);
    }
    
    @media (max-width: 600px) {
      body {
        padding: 1.2rem;
      }
      
      h1 {
        font-size: 2rem;
      }
      
      .routine-card {
        padding: 1.5rem;
      }
    }
  </style>
</head>
<body>
  <div class="container">
    <header>
      <button class="theme-toggle" id="themeToggle">🌓</button>
      <h1>My Routine ✨</h1>
      <p class="subtitle">personal skincare web</p>
      <div class="day-display" id="currentDayTime"></div>
    </header>
    
    <div id="faceRoutine" class="routine-card">
      <div class="routine-header">
        <div class="routine-icon" id="faceIcon">🌞</div>
        <div>
          <div class="routine-title" id="faceTitle">Morning Routine</div>
          <div class="routine-subtitle" id="faceSubtitle">Start your day with radiant skin</div>
        </div>
      </div>
      
      <div class="progress-container">
        <div class="progress-bar" id="faceProgress"></div>
      </div>
      <div class="progress-text"><span id="faceCompleted">0</span>/<span id="faceTotal">0</span> steps completed</div>
      
      <ul class="task-list" id="faceTaskList"></ul>
      
      <div class="tip-box">
        <div class="tip-title">Pro Tip</div>
        <div id="faceTip">Apply serums to damp skin for better absorption</div>
      </div>
    </div>
    
    <div id="handRoutine" class="routine-card">
      <div class="routine-header">
        <div class="routine-icon hand-icon">✋</div>
        <div>
          <div class="routine-title" id="handTitle">Hand Care</div>
          <div class="routine-subtitle" id="handSubtitle">Don't forget your hardworking hands</div>
        </div>
      </div>
      
      <div class="progress-container">
        <div class="progress-bar" id="handProgress"></div>
      </div>
      <div class="progress-text"><span id="handCompleted">0</span>/<span id="handTotal">0</span> steps completed</div>
      
      <ul class="task-list" id="handTaskList"></ul>
      
      <div class="tip-box">
        <div class="tip-title">Did You Know?</div>
        <div id="handTip">Hands show signs of aging faster than your face</div>
      </div>
    </div>
    
    <div id="completionMessage" class="completion-message">
      <div class="completion-title">All done! Great job! 🎉</div>
      <div class="completion-subtitle">Next up: <span id="nextRoutineName"></span></div>
      <div class="time-remaining">Time remaining: <span id="timeRemaining"></span></div>
    </div>
    
    <button class="refresh-btn" onclick="location.reload()">Refresh</button>
  </div>

  <script>
    // ========== THEME TOGGLE ==========
    const themeToggle = document.getElementById('themeToggle');
    const prefersDark = window.matchMedia('(prefers-color-scheme: dark)').matches;
    
    // Set initial theme
    if (localStorage.getItem('theme') === 'dark' || (!localStorage.getItem('theme') && prefersDark)) {
      document.body.classList.add('dark-mode');
      themeToggle.textContent = '🌞';
    } else {
      themeToggle.textContent = '🌙';
    }
    
    // Toggle theme
    themeToggle.addEventListener('click', () => {
      document.body.classList.toggle('dark-mode');
      const isDark = document.body.classList.contains('dark-mode');
      localStorage.setItem('theme', isDark ? 'dark' : 'light');
      themeToggle.textContent = isDark ? '🌞' : '🌙';
    });
    
    // ========== TIME DISPLAY ==========
    function updateDateTime() {
      const now = new Date();
      const dayName = now.toLocaleDateString('en-US', { weekday: 'long' });
      const dateString = now.toLocaleDateString('en-US', { 
        month: 'long', day: 'numeric', year: 'numeric' 
      });
      const timeString = now.toLocaleTimeString('en-US', { 
        hour: '2-digit', minute: '2-digit' 
      });
      
      document.getElementById('currentDayTime').textContent = 
        `${dayName}, ${dateString} • ${timeString}`;
    }
    
    // Update time immediately and every minute
    updateDateTime();
    setInterval(updateDateTime, 60000);
    
    // ========== ROUTINE LOGIC ==========
    const now = new Date();
    const currentHour = now.getHours();
    const isMorning = currentHour >= 6 && currentHour < 16;
    const dayName = now.toLocaleDateString('en-US', { weekday: 'long' });
    
    // Face care routines - generated from ESP32 configuration
)rawliteral";

    // Add the generated JavaScript for routines
    String morningFaceTasks[10];
    for (int i = 0; i < 10 && morningFaceRoutine.tasks[i][0] != '\0'; i++) {
      morningFaceTasks[i] = morningFaceRoutine.tasks[i];
    }
    html += generateJSRoutineObject("morningFaceRoutine", 
                                   morningFaceRoutine.title, 
                                   morningFaceRoutine.subtitle, 
                                   morningFaceTasks, 
                                   morningFaceRoutine.tip);

    // Generate night face routines
    String nightFaceTitles[7], nightFaceSubtitles[7], nightFaceTips[7];
    String nightFaceTasks[7][10];
    
    for (int day = 0; day < 7; day++) {
      nightFaceTitles[day] = nightFaceRoutines.weekday[day];
      nightFaceSubtitles[day] = nightFaceRoutines.subtitle[day];
      nightFaceTips[day] = nightFaceRoutines.tip[day];
      
      for (int i = 0; i < 10 && nightFaceRoutines.tasks[day][i][0] != '\0'; i++) {
        nightFaceTasks[day][i] = nightFaceRoutines.tasks[day][i];
      }
    }
    
    html += generateJSDayRoutineObject("nightFaceRoutines", 
                                     nightFaceTitles, 
                                     nightFaceSubtitles, 
                                     nightFaceTasks, 
                                     nightFaceTips);

    // Generate morning hand routine
    String morningHandTasks[10];
    for (int i = 0; i < 10 && morningHandRoutine.tasks[i][0] != '\0'; i++) {
      morningHandTasks[i] = morningHandRoutine.tasks[i];
    }
    html += generateJSRoutineObject("morningHandRoutine", 
                                   morningHandRoutine.title, 
                                   morningHandRoutine.subtitle, 
                                   morningHandTasks, 
                                   morningHandRoutine.tip);

    // Generate night hand routines
    String nightHandTitles[7], nightHandSubtitles[7], nightHandTips[7];
    String nightHandTasks[7][10];
    
    for (int day = 0; day < 7; day++) {
      nightHandTitles[day] = nightHandRoutines.weekday[day];
      nightHandSubtitles[day] = nightHandRoutines.subtitle[day];
      nightHandTips[day] = nightHandRoutines.tip[day];
      
      for (int i = 0; i < 10 && nightHandRoutines.tasks[day][i][0] != '\0'; i++) {
        nightHandTasks[day][i] = nightHandRoutines.tasks[day][i];
      }
    }
    
    html += generateJSDayRoutineObject("nightHandRoutines", 
                                     nightHandTitles, 
                                     nightHandSubtitles, 
                                     nightHandTasks, 
                                     nightHandTips);

    html += R"rawliteral(
    // Determine which routines to show
    const faceRoutine = isMorning ? morningFaceRoutine : nightFaceRoutines[dayName];
    const handRoutine = isMorning ? morningHandRoutine : nightHandRoutines[dayName];
    
    // Update UI with routines
    document.getElementById('faceIcon').textContent = isMorning ? '🌞' : '🌙';
    document.getElementById('faceTitle').textContent = faceRoutine.title;
    document.getElementById('faceSubtitle').textContent = faceRoutine.subtitle;
    document.getElementById('faceTip').textContent = faceRoutine.tip;
    
    document.getElementById('handTitle').textContent = handRoutine.title;
    document.getElementById('handSubtitle').textContent = handRoutine.subtitle;
    document.getElementById('handTip').textContent = handRoutine.tip;
    
    // Set next routine info
    const nextIsMorning = !isMorning;
    const nextRoutineName = nextIsMorning ? "Morning Glow" : "Night Care";
    document.getElementById('nextRoutineName').textContent = nextRoutineName;
    
    // Calculate time until next routine
    let hoursUntilNext;
    if (isMorning) {
      hoursUntilNext = 16 - currentHour; // Until 4pm
    } else {
      hoursUntilNext = (24 - currentHour) + 6; // Until next day 6am
    }
    
    const nextRoutineTime = isMorning ? "tonight" : "tomorrow morning";
    document.getElementById('timeRemaining').textContent = 
      `${hoursUntilNext} hours (${nextRoutineTime})`;
    
    // ========== TASK MANAGEMENT ==========
    function loadTasks(listId, routine, prefix) {
      const listElement = document.getElementById(listId);
      const totalTasks = routine.tasks.length;
      let completedTasks = 0;
      
      document.getElementById(`${prefix}Total`).textContent = totalTasks;
      
      routine.tasks.forEach((task, index) => {
        const li = document.createElement('li');
        li.className = 'task-item';
        
        const checkbox = document.createElement('input');
        checkbox.type = 'checkbox';
        checkbox.className = 'task-checkbox';
        checkbox.id = `${prefix}-task-${index}`;
        
        // Load saved state
        const isChecked = localStorage.getItem(`${prefix}-task-${index}`) === 'true';
        checkbox.checked = isChecked;
        if (isChecked) completedTasks++;
        
        checkbox.addEventListener('change', function() {
          localStorage.setItem(`${prefix}-task-${index}`, this.checked);
          updateProgress(prefix);
          checkAllDone();
        });
        
        const label = document.createElement('label');
        label.className = 'task-label';
        label.textContent = task;
        label.htmlFor = `${prefix}-task-${index}`;
        
        li.appendChild(checkbox);
        li.appendChild(label);
        listElement.appendChild(li);
      });
      
      // Initial progress update
      updateProgress(prefix);
    }
    
    function updateProgress(prefix) {
      const totalTasks = parseInt(document.getElementById(`${prefix}Total`).textContent);
      let completedTasks = 0;
      
      for (let i = 0; i < totalTasks; i++) {
        if (localStorage.getItem(`${prefix}-task-${i}`) === 'true') {
          completedTasks++;
        }
      }
      
      document.getElementById(`${prefix}Completed`).textContent = completedTasks;
      const progressPercent = (completedTasks / totalTasks) * 100;
      document.getElementById(`${prefix}Progress`).style.width = `${progressPercent}%`;
      
      return { completed: completedTasks, total: totalTasks };
    }
    
    function checkAllDone() {
      const faceProgress = updateProgress('face');
      const handProgress = updateProgress('hand');
      
      const allFaceDone = faceProgress.completed === faceProgress.total;
      const allHandDone = handProgress.completed === handProgress.total;
      
      if (allFaceDone && allHandDone) {
        document.getElementById('faceRoutine').style.display = 'none';
        document.getElementById('handRoutine').style.display = 'none';
        document.getElementById('completionMessage').style.display = 'block';
      } else {
        document.getElementById('faceRoutine').style.display = 'block';
        document.getElementById('handRoutine').style.display = 'block';
        document.getElementById('completionMessage').style.display = 'none';
      }
    }
    
    // Initialize both routines
    loadTasks('faceTaskList', faceRoutine, 'face');
    loadTasks('handTaskList', handRoutine, 'hand');
    
    // Initial check if all done
    checkAllDone();
  </script>
</body>
</html>
)rawliteral";

    request->send(200, "text/html", html);
  });

  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection lost. Reconnecting...");
    connectToWiFi();
  }
  
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 60000) {
    lastCheck = millis();
    Serial.println("System is running...");
  }
}
