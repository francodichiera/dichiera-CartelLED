"use client"

import { useState, useEffect } from "react"
import { Button } from "./components/ui/button"
import { Input } from "./components/ui/input"
import { Label } from "./components/ui/label"
import { RadioGroup, RadioGroupItem } from "./components/ui/radio-group"
import { Slider } from "./components/ui/slider"
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from "./components/ui/card"
import { Send, ArrowLeft, ArrowRight, RotateCcw, Zap, Activity } from "lucide-react"
import { EnhancedRgbLedIcon } from "./components/enhanced-rgb-led-icon"

export default function IntelligentSignController() {
  const [message, setMessage] = useState("")
  const [invert, setInvert] = useState("0")
  const [scrollType, setScrollType] = useState("L")
  const [speed, setSpeed] = useState([100])
  const [isConnected, _setIsConnected] = useState(true)
  const [glitchActive, setGlitchActive] = useState(false)

  useEffect(() => {
    const glitchInterval = setInterval(() => {
      setGlitchActive(true)
      setTimeout(() => setGlitchActive(false), 300)
    }, 8000)
    return () => clearInterval(glitchInterval)
  }, [])

  const sendData = () => {
    const msg = encodeURIComponent(message)
    const SD = scrollType
    const I = invert
    const SP = speed[0].toString()

    setGlitchActive(true)
    setTimeout(() => setGlitchActive(false), 500)

    window.location.href = `/?MSG=${msg}&SD=${SD}&I=${I}&SP=${SP}`
  }

  const getSpeedLabel = (value: number) => {
    if (value <= 50) return "RÁPIDO"
    if (value <= 150) return "MEDIO"
    return "LENTO"
  }

  return (
    <div className="min-h-screen bg-gradient-to-br from-gray-900 via-blue-950 to-black p-4 relative overflow-hidden">
      <div className="fixed top-4 left-4 z-50">
        <img
          src="https://hebbkx1anhila5yf.public.blob.vercel-storage.com/image-removebg-preview-kR531p9m5VmcYBeG03P2jNWpQ9dkn8.png"
          alt="Huergo Logo"
          className="h-16 w-auto opacity-90 hover:opacity-100 transition-opacity duration-300"
        />
      </div>

      <div className="absolute inset-0 opacity-5">
        <div className="absolute top-0 left-0 w-full h-0.5 bg-gradient-to-r from-transparent via-blue-500 to-transparent scan-line" />
        {Array.from({ length: 20 }).map((_, i) => (
          <div
            key={i}
            className="absolute w-px h-4 bg-green-400 matrix-rain"
            style={{
              left: `${Math.random() * 100}%`,
              animationDelay: `${Math.random() * 4}s`,
            }}
          />
        ))}
      </div>

      <div className="fixed top-4 right-4 z-50">
        <div
          className={`flex items-center gap-2 px-4 py-2 rounded-full text-sm font-bold tracking-wider ${
            isConnected
              ? "bg-green-500/20 text-green-400 border border-green-500/30"
              : "bg-red-500/20 text-red-400 border border-red-500/30"
          }`}
        >
          <div className={`w-2 h-2 rounded-full ${isConnected ? "bg-green-400" : "bg-red-400"} animate-pulse`} />
          {isConnected ? "CONECTADO" : "DESCONECTADO"}
        </div>
      </div>

      <div className="max-w-2xl mx-auto relative z-10">
        <div className="text-center mb-8">
          <div className="inline-flex items-center justify-center w-24 h-24 bg-gradient-to-r from-blue-800/20 to-purple-800/20 rounded-full mb-6 neon-glow backdrop-blur-sm border border-blue-500/30">
            <EnhancedRgbLedIcon />
          </div>
          <h1 className={`text-5xl font-black tracking-wider mb-3 ${glitchActive ? "glitch-effect" : ""}`}>
            <span className="bg-gradient-to-r from-blue-400 via-purple-400 to-blue-400 bg-clip-text text-transparent font-black">
              CARTEL INTELIGENTE
            </span>
          </h1>
          <p className="text-gray-300 text-lg tracking-widest font-semibold">
            CONTROLA TU PANTALLA LED CON ESTILO Y PRECISIÓN
          </p>

          <div className="flex justify-center gap-6 mt-4">
            <div className="flex items-center gap-2 text-sm text-gray-400">
              <Activity className="w-4 h-4 text-green-400" />
              <span className="tracking-wider">SISTEMA ACTIVO</span>
            </div>
            <div className="flex items-center gap-2 text-sm text-gray-400">
              <Zap className="w-4 h-4 text-yellow-400" />
              <span className="tracking-wider">ENERGÍA ÓPTIMA</span>
            </div>
          </div>
        </div>

        <Card className="shadow-2xl border-0 bg-gray-900/95 backdrop-blur-md border border-blue-800/40 neon-glow">
          <CardHeader className="text-center pb-8">
            <CardTitle className="text-3xl font-black text-white tracking-wider">PANEL DE CONTROL</CardTitle>
            <CardDescription className="text-gray-300 text-lg tracking-wide">
              CONFIGURA TU MENSAJE Y OPCIONES DE VISUALIZACIÓN
            </CardDescription>
          </CardHeader>

          <CardContent className="space-y-10">
            <div className="space-y-4">
              <Label htmlFor="message" className="text-xl font-bold text-gray-200 tracking-wider">
                MENSAJE
              </Label>
              <div className="relative">
                <Input
                  id="message"
                  type="text"
                  placeholder="ESCRIBE TU MENSAJE AQUÍ..."
                  value={message}
                  onChange={(e) => setMessage(e.target.value.toUpperCase())}
                  maxLength={255}
                  className="text-xl h-14 border-2 border-gray-600 focus:border-blue-400 bg-gray-800/80 text-white placeholder:text-gray-500 transition-all duration-300 tracking-wider font-bold"
                />
                <div className="absolute -bottom-6 right-0 text-sm font-bold tracking-wider">
                  <span className={message.length > 200 ? "text-red-400" : "text-blue-400"}>{message.length}</span>
                  <span className="text-gray-500">/255 CARACTERES</span>
                </div>
              </div>
            </div>

            <div className="space-y-6">
              <Label className="text-xl font-bold text-gray-200 tracking-wider">MODO DE VISUALIZACIÓN</Label>
              <RadioGroup value={invert} onValueChange={setInvert} className="grid grid-cols-2 gap-6">
                <div className="flex items-center space-x-4 p-6 border-2 border-gray-600 rounded-xl hover:border-blue-400 bg-gray-800/60 transition-all duration-300 hover:bg-gray-800/80">
                  <RadioGroupItem value="0" id="normal" className="w-5 h-5" />
                  <Label
                    htmlFor="normal"
                    className="flex items-center gap-3 cursor-pointer text-gray-200 font-bold tracking-wider"
                  >
                    <div className="w-8 h-8 bg-gray-700 rounded-full flex items-center justify-center border-2 border-gray-500">
                      <div className="w-3 h-3 bg-white rounded-full"></div>
                    </div>
                    NORMAL
                  </Label>
                </div>
                <div className="flex items-center space-x-4 p-6 border-2 border-gray-600 rounded-xl hover:border-blue-400 bg-gray-800/60 transition-all duration-300 hover:bg-gray-800/80">
                  <RadioGroupItem value="1" id="inverse" className="w-5 h-5" />
                  <Label
                    htmlFor="inverse"
                    className="flex items-center gap-3 cursor-pointer text-gray-200 font-bold tracking-wider"
                  >
                    <RotateCcw className="w-8 h-8 text-purple-400" />
                    INVERSA
                  </Label>
                </div>
              </RadioGroup>
            </div>

            <div className="space-y-6">
              <Label className="text-xl font-bold text-gray-200 tracking-wider">DIRECCIÓN DE DESPLAZAMIENTO</Label>
              <RadioGroup value={scrollType} onValueChange={setScrollType} className="grid grid-cols-2 gap-6">
                <div className="flex items-center space-x-4 p-6 border-2 border-gray-600 rounded-xl hover:border-blue-400 bg-gray-800/60 transition-all duration-300 hover:bg-gray-800/80">
                  <RadioGroupItem value="L" id="left" className="w-5 h-5" />
                  <Label
                    htmlFor="left"
                    className="flex items-center gap-3 cursor-pointer text-gray-200 font-bold tracking-wider"
                  >
                    <ArrowLeft className="w-8 h-8 text-blue-400 animate-pulse" />
                    IZQUIERDA
                  </Label>
                </div>
                <div className="flex items-center space-x-4 p-6 border-2 border-gray-600 rounded-xl hover:border-blue-400 bg-gray-800/60 transition-all duration-300 hover:bg-gray-800/80">
                  <RadioGroupItem value="R" id="right" className="w-5 h-5" />
                  <Label
                    htmlFor="right"
                    className="flex items-center gap-3 cursor-pointer text-gray-200 font-bold tracking-wider"
                  >
                    <ArrowRight className="w-8 h-8 text-blue-400 animate-pulse" />
                    DERECHA
                  </Label>
                </div>
              </RadioGroup>
            </div>

            <div className="space-y-6">
              <Label className="text-xl font-bold text-gray-200 tracking-wider">VELOCIDAD DE DESPLAZAMIENTO</Label>
              <div className="px-6 py-4 bg-gray-800/40 rounded-xl border border-gray-700">
                <Slider value={speed} onValueChange={setSpeed} max={200} min={10} step={10} className="w-full mb-4" />
                <div className="flex justify-between text-sm font-bold tracking-wider">
                  <span className="text-green-400">RÁPIDO</span>
                  <span className="text-2xl font-black text-blue-400">
                    {getSpeedLabel(speed[0])} ({speed[0]})
                  </span>
                  <span className="text-red-400">LENTO</span>
                </div>
              </div>
            </div>

            <Button
              onClick={sendData}
              disabled={!message.trim()}
              className="w-full h-16 text-xl font-black tracking-widest bg-gradient-to-r from-blue-600 via-purple-600 to-blue-600 hover:from-blue-700 hover:via-purple-700 hover:to-blue-700 transition-all duration-300 shadow-2xl hover:shadow-blue-500/25 disabled:opacity-50 disabled:cursor-not-allowed transform hover:scale-105 active:scale-95 neon-glow"
            >
              <Send className="w-6 h-6 mr-3" />
              ENVIAR MENSAJE
            </Button>
          </CardContent>
        </Card>

        <Card className="mt-8 shadow-2xl border-0 bg-gray-800/90 backdrop-blur-md border border-blue-700/40">
          <CardHeader>
            <CardTitle className="text-2xl font-black text-white tracking-wider">VISTA PREVIA</CardTitle>
          </CardHeader>
          <CardContent>
            <div className="bg-black rounded-xl p-8 min-h-[120px] flex items-center justify-center relative overflow-hidden border-2 border-gray-700">
              <div className="absolute inset-0 opacity-30">
                <div className="grid grid-cols-24 grid-rows-10 gap-px h-full w-full p-4">
                  {Array.from({ length: 240 }).map((_, i) => (
                    <div
                      key={i}
                      className="w-1 h-1 rounded-full bg-green-400 led-pulse"
                      style={{
                        animationDelay: `${(i * 0.02) % 3}s`,
                        opacity: Math.random() > 0.7 ? 0.8 : 0.2,
                      }}
                    />
                  ))}
                </div>
              </div>

              <div
                className={`text-3xl font-black relative z-10 tracking-widest ${
                  invert === "1"
                    ? "text-black bg-white px-4 py-2 rounded-lg shadow-lg"
                    : "text-green-400 drop-shadow-lg"
                } ${glitchActive ? "glitch-effect" : ""}`}
                style={{
                  textShadow: invert === "0" ? "0 0 20px #22c55e" : "none",
                }}
              >
                {message || "TU MENSAJE APARECERÁ AQUÍ..."}
              </div>
            </div>

            <div className="mt-6 text-center">
              <div className="inline-flex items-center gap-4 px-6 py-3 bg-gray-900/60 rounded-full border border-gray-600">
                <span className="text-sm font-bold text-blue-400 tracking-wider">
                  DIRECCIÓN: {scrollType === "L" ? "IZQUIERDA" : "DERECHA"}
                </span>
                <div className="w-px h-4 bg-gray-600" />
                <span className="text-sm font-bold text-purple-400 tracking-wider">
                  VELOCIDAD: {getSpeedLabel(speed[0])}
                </span>
                <div className="w-px h-4 bg-gray-600" />
                <span className="text-sm font-bold text-green-400 tracking-wider">
                  MODO: {invert === "0" ? "NORMAL" : "INVERSO"}
                </span>
              </div>
            </div>
          </CardContent>
        </Card>

        <div className="mt-8 text-center">
          <p className="text-gray-400 text-sm tracking-widest font-semibold">
            REALIZADO POR <span className="text-blue-400 font-bold">FRANCO DICHIERA</span>
          </p>
        </div>
      </div>
    </div>
  )
}
