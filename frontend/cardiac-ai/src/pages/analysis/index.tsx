import {
	Box,
	Button,
	FormControlLabel,
	Switch,
	TextField,
	Typography,
	useMediaQuery,
	useTheme,
} from "@mui/material";
import React, { useRef } from "react";
import { useAuth } from "../../context/AuthProvider";
import {
	Chart,
	LineController,
	LineElement,
	PointElement,
	LinearScale,
	CategoryScale,
	Title,
	Tooltip,
} from "chart.js";
import { saveDeviceReadings } from "../../services/readings";

// Register required components
Chart.register(
	LineController,
	LineElement,
	PointElement,
	LinearScale,
	CategoryScale,
	Title,
	Tooltip
);

const Analysis = () => {
	const { deviceMetadata } = useAuth();
	const theme = useTheme();
	const canvasRef = useRef<HTMLCanvasElement | null>(null);
	const chartRef = useRef<Chart | null>(null);

	const isMobile = useMediaQuery(theme.breakpoints.down("sm"));
	const isTablet = useMediaQuery(theme.breakpoints.between("sm", "md"));
	const biggerTablet = useMediaQuery(theme.breakpoints.between("md", "lg"));

	const [logMessage, setLogMessage] = React.useState<string>("");
	const [saveReadings, setSaveReadings] = React.useState<boolean>(false);

	const canvasWidth = isMobile
		? 320
		: isTablet
		? 400
		: biggerTablet
		? 600
		: 800;

	const canvasHeight = isMobile ? 250 : biggerTablet ? 280 : 300;

	const deviceId = deviceMetadata?.device_id;
	const socket = new WebSocket(
		`ws://localhost:8000/api/ws/frontend?device_id=${deviceId}`
	);

	const MAX_POINTS = 40;

	const handleSaveReadings = async (enable: boolean) => {
		const token = localStorage.getItem("cardiac_ai_access_token") || "";
		const deviceId = localStorage.getItem("cardiac_ai_device_id") || "";
		try {
			await saveDeviceReadings(deviceId, token, enable);
		} catch (error) {
			console.error("Error saving readings:", error);
		}
	};

	React.useEffect(() => {
		handleSaveReadings(saveReadings);
	}, [saveReadings]);

	React.useEffect(() => {
		if (!canvasRef.current) return;

		const ctx = canvasRef.current.getContext("2d");
		if (!ctx) return;

		if (chartRef.current) {
			chartRef.current.destroy();
		}

		const ecgChart = new Chart(ctx, {
			type: "line",
			data: {
				labels: Array(MAX_POINTS).fill(""),
				datasets: [
					{
						label: "ECG Signal",
						borderColor: "red",
						backgroundColor: "rgba(255,0,0,0.1)",
						data: [] as number[],
						pointRadius: 0,
						borderWidth: 2,
						tension: 0.3,
					},
				],
			},
			options: {
				animation: false,
				responsive: true,
				scales: {
					x: { display: false },
					y: {
						min: -1,
						max: 1,
						title: { display: true, text: "Amplitude" },
					},
				},
			},
		});

		chartRef.current = ecgChart;
		return () => {
			if (chartRef.current) {
				chartRef.current.destroy();
				chartRef.current = null;
			}
		};
	}, []);

	socket.onopen = () => {
		setLogMessage("Connected to backend");
	};

	socket.onmessage = (event) => {
		const rawValue = event.data.trim();
		const ecgPoint = parseFloat(rawValue);

		if (!isNaN(ecgPoint)) {
			if (chartRef.current) {
				const dataSet = chartRef.current.data.datasets[0].data as number[];
				dataSet.push(ecgPoint);
				if (dataSet.length > MAX_POINTS) {
					dataSet.shift();
				}
				chartRef.current.update("none");
			}
		} else {
			console.warn("Invalid ECG data:", rawValue);
		}
	};

	socket.onclose = () => {
		setLogMessage("Connection closed");
	};
	return (
		<Box
			sx={{
				width: "100vw",
				height: "calc(100vh - 4rem)",
				display: "flex",
				flexDirection: "column",
				justifyContent: "center",
				alignItems: "center",
			}}
		>
			<Box
				sx={{
					background: "rgba(249, 250, 255, 1)",
					// width: { xs: "80%", sm: "70%" },
					// height: "70%",
					width: "calc(100% - 6rem)",
					height: "calc(100% - 6rem)",
					display: "flex",
					flexDirection: "column",
					padding: "3rem",
					borderRadius: "1rem",
				}}
			>
				<Box
					sx={{
						display: "flex",
						flexDirection: "row",
						justifyContent: "space-between",
						alignItems: "center",
					}}
				>
					<FormControlLabel
						control={
							<Switch
								checked={saveReadings}
								sx={{ zIndex: 1 }}
								onClick={() => setSaveReadings((prev) => !prev)}
							/>
						}
						label="Save Data"
					/>
					<Typography>Device ID: <b>{deviceId}</b></Typography>
					<Typography>Session: <b>01:23</b></Typography>
					<Typography>1000 data points: 39 KB</Typography>
				</Box>
				<Box sx={{ width: "100%", margin: "1rem 0" }}>
					<canvas
						id="analysisCanvas"
						ref={canvasRef}
						width={canvasWidth}
						height={canvasHeight}
						style={{
							border: "1px solid #000",
							borderRadius: "1rem",
							background: "#fff",
						}}
					/>
					<pre id="log">{logMessage}</pre>
				</Box>
				<Box
					sx={{
						display: "flex",
						flexDirection: { xs: "column", sm: "row" },
						justifyContent: "space-evenly",
						padding: "1.5rem",
						gap: "1rem",
					}}
				>
					<Button
						variant="outlined"
						sx={{
							border: "1px solid rgba(47, 90, 196, 1)",
							padding: "1rem 2rem",
							background: "#fff",
							borderRadius: "1rem",
							":hover": {
								border: "2px solid rgba(47, 90, 196, 1)",
								background: "rgba(47, 90, 196, 1)",
								color: "#fff",
							},
						}}
					>
						Download Previous
					</Button>
					<Button
						variant="outlined"
						sx={{
							border: "1px solid rgba(47, 90, 196, 1)",
							padding: "1rem 2rem",
							background: "#fff",
							borderRadius: "1rem",
							":hover": {
								border: "2px solid rgba(47, 90, 196, 1)",
								background: "rgba(47, 90, 196, 1)",
								color: "#fff",
							},
						}}
					>
						Download Current
					</Button>
					<Button
						variant="outlined"
						sx={{
							border: "1px solid rgba(47, 90, 196, 1)",
							padding: "1rem 2rem",
							background: "#fff",
							borderRadius: "1rem",
							":hover": {
								border: "2px solid rgba(47, 90, 196, 1)",
								background: "rgba(47, 90, 196, 1)",
								color: "#fff",
							},
						}}
					>
						Request AI Analysis
					</Button>
				</Box>
				<Box sx={{ padding: { xs: "0", sm: "1.5rem" } }}>
					<TextField
						placeholder="AI Analysis will be displayed here..."
						multiline
						rows={5}
						sx={{
							width: { xs: "100%", sm: "90%" },
							background: "#fff",
							"& .MuiInputBase-root": {
								borderRadius: "1rem",
							},
						}}
					/>
				</Box>
			</Box>
		</Box>
	);
};

export default Analysis;
