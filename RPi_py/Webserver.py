from flask import Flask, render_template_string, jsonify

app = Flask(__name__)

# --- Configuration ---
# You can change the port here if needed
PORT = 5000

# --- HTML/CSS/JS Template ---
# We embed this here to keep it in one file, but usually, this goes in a 'templates' folder.
HTML_TEMPLATE = """
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>RPi Controller</title>
    <style>
        body {
            background-color: #1a1a1a;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            color: white;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            height: 100vh;
            margin: 0;
        }

        h1 { margin-bottom: 2rem; letter-spacing: 2px; }

        .button-container {
            display: flex;
            gap: 20px;
            flex-direction: column;
        }

        button {
            padding: 20px 60px;
            font-size: 1.5rem;
            font-weight: bold;
            border: none;
            border-radius: 12px;
            cursor: pointer;
            transition: transform 0.1s, box-shadow 0.2s, filter 0.2s;
            color: white;
            text-transform: uppercase;
            box-shadow: 0 4px 6px rgba(0,0,0,0.3);
        }

        button:active {
            transform: translateY(2px);
            box-shadow: 0 1px 2px rgba(0,0,0,0.3);
        }

        /* Specific Button Styles */
        #btn-up {
            background: linear-gradient(145deg, #00b09b, #96c93d);
        }
        
        #btn-down {
            background: linear-gradient(145deg, #ff5f6d, #ffc371);
        }

        /* Hover Glow */
        button:hover {
            filter: brightness(1.1);
        }

        /* Feedback Log */
        #log {
            margin-top: 2rem;
            color: #888;
            font-size: 0.9rem;
            min-height: 1.2em;
        }
    </style>
</head>
<body>

    <h1>MTMS Robot</h1>

    <div class="button-container">
        <button id="btn-up" onclick="sendCommand('UP')">▲ UP</button>
        <button id="btn-down" onclick="sendCommand('DOWN')">▼ DOWN</button>
    </div>

    <div id="log">Ready...</div>

    <script>
        function sendCommand(direction) {
            // Update UI immediately
            const log = document.getElementById('log');
            log.innerText = `Sending ${direction}...`;

            // Send request to Python backend
            fetch('/click/' + direction, { method: 'POST' })
                .then(response => {
                    if (response.ok) {
                        log.innerText = `Executed: ${direction}`;
                        console.log(`${direction} sent successfully`);
                    } else {
                        log.innerText = "Error sending command";
                    }
                })
                .catch(error => {
                    log.innerText = "Connection lost";
                    console.error('Error:', error);
                });
        }
    </script>
</body>
</html>
"""

@app.route('/')
def home():
    """Serves the webpage."""
    return render_template_string(HTML_TEMPLATE)

@app.route('/click/<action>', methods=['POST'])
def handle_click(action):
    """Handles the button clicks."""
    action = action.upper()
    
    # --- YOUR CUSTOM CODE GOES HERE ---
    if action == "UP":
        print(">>> USER CLICKED: UP")
        # Example: GPIO.output(17, GPIO.HIGH)
        
    elif action == "DOWN":
        print(">>> USER CLICKED: DOWN")
        # Example: GPIO.output(17, GPIO.LOW)
    # ----------------------------------

    return jsonify(success=True)

if __name__ == '__main__':
    # host='0.0.0.0' allows access from other devices on the network
    print(f"Server starting at http://0.0.0.0:{PORT}")
    app.run(host='0.0.0.0', port=PORT, debug=True)