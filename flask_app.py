
from flask import (
    Flask, redirect,
    url_for, request, jsonify,
    render_template, session
)
from functools import wraps


app = Flask(__name__)
app.secret_key = 'climatemonitoring-vivero@!@#'

temperature, humidity, light = float(), float(), int()

@app.errorhandler(404)
def not_found(e):
    return '404 : Not Found'

def requiered(f):
    @wraps(f)
    def wrapper(*args, **kwargs):
        if session.get('logged') == True:
            return f(*args, **kwargs)
        else:
            return redirect(url_for('unlock'))
    return wrapper

@app.route('/', methods=['POST', 'GET'])
def unlock():
    args = None
    if request.method == 'POST':
        codigo = request.form.get('codigo')
        if codigo == '182301P':           
            session['logged'] = True
            return redirect(url_for('home')) 
        args = False
    return render_template('lock.html', args=args)

    
@app.route('/home', methods=['POST', 'GET'])
@requiered
def home():
    global temperature, humidity, light
    return render_template("home.html",
                          temperature=temperature,
                          humidity=humidity,
                          light=light,
                          activeH="active",
                          activeT="")

@app.route('/team', methods=['POST', 'GET'])
@requiered
def team():
    return render_template("team.html", activeH="",
                          activeT="active")

@app.route('/update', methods=['POST'])
def update():

    global temperature, humidity, light
    return jsonify({
        "temperature" : temperature,
        "humidity" : humidity,
        "light" :  light
    })

@app.route('/record', methods=['POST', 'GET'])
def record():
    response = {"status": 0}
    try:
        global temperature, humidity, light
        content = request.get_json()
        temperature = round(content.get("temperature"),2)
        humidity = round(content.get("humidity"),2)
        light = round(content.get("light"),2)
        response["status"] = 1

    except Exception as e:
        response = {"status": -1}

    return response


if __name__ == '__name__':
    app.run(port=8000)
