print("run lua init.lua")

function OnInit(id)
	sunnet.NewService("chat")
--[[	print("[lua] main OnInit id: "..id)
	local ping1 = sunnet.NewService("ping");
	print("[lua] new service ping1: "..ping1)

	local ping2 = sunnet.NewService("ping")
	print("[lua] new service ping2: "..ping2)

	local pong = sunnet.NewService("ping")
	print("[lua] new service pong: "..pong)

	sunnet.Send(ping1,pong,"start")
	sunnet.Send(ping2,pong,"start")
	]]
end

function OnExit()
	print("[lua] main OnExit")
end
