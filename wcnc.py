# -- coding: utf-8 --
from flask import Flask, render_template, request, redirect, url_for
from werkzeug.utils import secure_filename
from cnc import Cnc
import sys, os

app = Flask(__name__)

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/cakes')
def cakes():
    return 'Yummy cakes!'
    
@app.route('/hello/')
@app.route('/hello/<name>')
def hello(name='world'):
    return render_template('page.html', name=name)
    
@app.route('/post/<int:post_id>')
def show_port(post_id):
    return 'Post %d' % post_id
    
@app.route('/move', methods=['POST', 'GET'])
def move():
    if request.method == 'POST':
        x=y=z=0
        if 'x' in request.form and request.form['x']:
            x = int(request.form['x'])
        if 'y' in request.form and request.form['y']:
            y = int(request.form['y'])
        if 'z' in request.form and request.form['z']: 
            z = int(request.form['z'])
        output = 'x=%d y=%d z=%d'% (x, y, z)
        cnc = Cnc('/dev/ttyUSB0')
        cnc.move(x, y, z)
        return output
    return render_template('move.html')
    
@app.route('/sand', methods=['POST', 'GET'])
def sand():
    return render_template('sand.html')
    
@app.route('/command', methods=['POST', 'GET'])
def command():
    if request.method == 'POST':
        cnc = Cnc('/dev/ttyUSB0')
        cnc.write(request.form['gcode'])
#        return 'ok'
    return render_template('command.html')
    
@app.route('/file', methods=['GET', 'POST'])
def file():
    if request.method == 'POST':
        if 'file' not in request.files:
            flash('No file part')
            return redirect(request.url)
        f = request.files['file']
        if f.filename == '':
            flash('No selected file')
            return redirect(request.url)
        cnc = Cnc('/dev/ttyUSB0')
        for line in f:
            cnc.write(line)
        return f.filename

    return render_template('file.html')
    
#if __name__ == '__main__':
    #app.run(debug=True, host='0.0.0.0')
  
