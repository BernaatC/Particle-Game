data = []
with open("badapple.xml") as file:
    data = file.readlines()

for i in range(len(data)):
    data[i] = data[i].replace("stage", "s")
    data[i] = data[i].replace("points", "p")
    data[i] = data[i].replace("movingParticles", "m")
    data[i] = data[i].replace("goals", "g")
    data[i] = data[i].replace("obstacles", "o")
    data[i] = data[i].replace("magneticFields", "f")
    data[i] = data[i].replace("animations", "a")
    data[i] = data[i].replace("particle", "p")
    data[i] = data[i].replace("goal", "g")
    data[i] = data[i].replace("rectangle", "r")
    data[i] = data[i].replace("upperLeftCorner", "u")
    data[i] = data[i].replace("size", "s")
    data[i] = data[i].replace("angle", "a")
    data[i] = data[i].replace("enabled", "e")
    data[i] = data[i].replace("id", "i")
    data[i] = data[i].replace("yes", "y")
    data[i] = data[i].replace("toggle", "t")
    data[i] = data[i].replace("time", "t")
    data[i] = data[i].replace("loopEvery", "l")
    data[i] = data[i].replace(" />", ">")
    data[i] = data[i].replace("pos", "p")
    data[i] = data[i].replace("charge", "c")
    data[i] = data[i].replace("radius", "r")

with open("badapple_c.xml", "w") as file:
    for line in data:
        file.write(line)