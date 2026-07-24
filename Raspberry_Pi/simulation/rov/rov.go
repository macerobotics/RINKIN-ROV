package rov

import (
	"fmt"
	"math"
	"os"
	"regexp"
	"strconv"
	"time"

	"github.com/ungerik/go3d/quaternion"
	"github.com/ungerik/go3d/vec3"
)

type EulerAngles struct {
	Heading, Pitch, Roll float32
}

type ROVInfo struct {
	Position    [3]float32
	Orientation EulerAngles
}

type Motor struct {
	position    vec3.T
	orientation vec3.T
	speed       float32
}

func (m Motor) getForce() vec3.T {
	return m.orientation.Scaled(0.01 * m.speed)
}

func newMotor(position, orientation vec3.T) Motor {
	return Motor{
		position:    position,
		orientation: orientation,
		speed:       0,
	}
}

type ROV struct {
	position                  vec3.T
	velocity                  vec3.T
	mass                      float32
	centerOfMass              vec3.T
	centerOfBuyoancy          vec3.T
	orientation               quaternion.T
	angularVelocity           vec3.T
	angularMomentum           vec3.T
	motors                    [5]Motor
	Info                      chan ROVInfo
	Cmd                       chan string
	dt                        time.Duration
	momentOfInertia           vec3.T
	sumOfForces, sumOfTorques vec3.T
}

func New() *ROV {
	r := &ROV{
		position:         vec3.T{0, 0, 0},
		velocity:         vec3.T{0, 0, 0},
		mass:             1,
		centerOfMass:     vec3.T{0, 0, 0},
		centerOfBuyoancy: vec3.T{0, 0, 0},
		orientation:      quaternion.T{0, 0, 0, 1},
		angularVelocity:  vec3.T{0, 0, 0},
		angularMomentum:  vec3.T{0, 0, 0},
		motors: [5]Motor{
			newMotor(vec3.T{0, 0.112, 0.230}, vec3.T{0, 1, 0}),
			newMotor(vec3.T{-0.120, 0, -0.041}, vec3.T{0, 0, 1}),
			newMotor(vec3.T{0.120, 0, -0.041}, vec3.T{0, 0, 1}),
			newMotor(vec3.T{-0.120, 0.102, -0.117}, vec3.T{0, 1, 0}),
			newMotor(vec3.T{0.120, 0.102, -0.117}, vec3.T{0, 1, 0}),
		},
		Info:            make(chan ROVInfo),
		Cmd:             make(chan string),
		dt:              1 * time.Millisecond,
		momentOfInertia: vec3.T{1, 1, 1},
		sumOfForces:     vec3.T{0, 0, 0},
		sumOfTorques:    vec3.T{0, 0, 0},
	}
	return r
}

func (r *ROV) Start() {
	fmt.Println("Start")
	go func() {
		r.loop()
	}()
}

func (r *ROV) loop() {
	fmt.Println("loop()")
	for {
		select {
		case cmd := <-r.Cmd:
			fmt.Println("received command in loop")
			r.runCmd(cmd)
		default:
		}
		r.sumOfForces = vec3.T{0, 0, 0}
		r.sumOfTorques = vec3.T{0, 0, 0}

		dt := float32(r.dt.Seconds())

		r.applyForce(vec3.T{0, -9.81 * r.mass, 0}, r.centerOfMass)
		r.applyForce(vec3.T{0, 9.81 * r.mass, 0}, r.centerOfBuyoancy)
		for _, m := range r.motors {
			r.applyForce(r.toWorld(m.getForce()), m.position)
		}

		acceleration := r.sumOfForces.Scaled(1 / r.mass * dt)
		r.velocity.Add(&acceleration)
		velocity := r.velocity.Scaled(dt)
		r.position.Add(&velocity)

		// https://gafferongames.com/post/physics_in_3d/

		dL := r.sumOfTorques.Scaled(dt)
		r.angularMomentum.Add(&dL)

		invertedInertia := vec3.T{1 / r.momentOfInertia[0], 1 / r.momentOfInertia[1], 1 / r.momentOfInertia[2]}
		angularVelocity := r.angularMomentum.Muled(&invertedInertia)
		r.orientation.Normalize()
		w := quaternion.T{angularVelocity[0], angularVelocity[1], angularVelocity[2], 0}
		spin := quaternion.MulRaw(&w, &r.orientation)
		spin[0] *= 0.5
		spin[1] *= 0.5
		spin[2] *= 0.5
		spin[3] *= 0.5

		dq := spin
		dq[0] *= dt
		dq[1] *= dt
		dq[2] *= dt
		dq[3] *= dt

		r.orientation[0] += dq[0]
		r.orientation[1] += dq[1]
		r.orientation[2] += dq[2]
		r.orientation[3] += dq[3]

		r.orientation.Normalize()

		heading, pitch, roll := r.orientation.ToEulerAngles()
		heading *= 180 / math.Pi
		pitch *= 180 / math.Pi
		roll *= 180 / math.Pi

		select {
		case r.Info <- ROVInfo{
			//Position: r.position.Scaled(1000),
			Position: vec3.T{0, 0, 0},
			Orientation: EulerAngles{
				Heading: heading,
				Pitch:   pitch,
				Roll:    roll,
			},
		}:
		default:
		}
		fmt.Println("position: ", r.position)
		fmt.Println("velocity: ", r.velocity)
		fmt.Println("quaternion: ", r.orientation)
		fmt.Println("orientation: ", heading, pitch, roll)
		fmt.Println("angular momentum: ", r.angularMomentum)
		time.Sleep(10 * r.dt)
	}
}

func (r *ROV) applyForce(f, pos vec3.T) {
	r.sumOfForces.Add(&f)
	_r := pos.Subed(&r.centerOfMass)
	torque := vec3.Cross(&_r, &f)
	r.sumOfTorques.Add(&torque)
}

func (r *ROV) toWorld(v vec3.T) vec3.T {
	return r.orientation.RotatedVec3(&v)
}

func (r *ROV) runCmd(cmd string) {
	regex, _ := regexp.Compile(`#(\d+)m(-?\d+)!\n`)

	l := regex.FindStringSubmatch(cmd)
	if l == nil {
		fmt.Fprintln(os.Stderr, "received invalid command: ", cmd)
	}
	fmt.Println(l)
	if len(l) == 3 {
		motor, err := strconv.Atoi(l[1])
		if err != nil || motor < 0 || motor >= len(r.motors) {
			fmt.Fprintln(os.Stderr, "invalid motor")
			return
		}
		speed, err := strconv.Atoi(l[2])
		if err != nil {
			fmt.Fprintln(os.Stderr, "invalid motor speed")
			return
		}
		r.motors[motor].speed = float32(speed)
		fmt.Printf("motor=%d speed = %d\n", motor, speed)
	}

}
