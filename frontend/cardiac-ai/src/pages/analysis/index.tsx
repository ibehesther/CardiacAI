import {
	Box,
	Button,
	CircularProgress,
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
import { differenceInSeconds, format } from "date-fns";
import { requestAIAnalysis, saveDeviceReadings } from "../../services/readings";
import PrevReadingModal from "./prevReadingModal";
import { getDeviceMetadata } from "../../services/auth";

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

enum ECGClasses {
	F = "F",
	M = "M",
	N = "N",
	Q = "Q",
	S = "S",
	V = "V",
}

const Analysis = () => {
	const { deviceMetadata, userRole } = useAuth();
	const theme = useTheme();
	const canvasRef = useRef<HTMLCanvasElement | null>(null);
	const chartRef = useRef<Chart | null>(null);

	const isMobile = useMediaQuery(theme.breakpoints.down("sm"));
	const isTablet = useMediaQuery(theme.breakpoints.between("sm", "md"));
	const biggerTablet = useMediaQuery(theme.breakpoints.between("md", "lg"));

	const [loading, setLoading] = React.useState<boolean>(false);
	const [openSocket, setOpenSocket] = React.useState<boolean>(true);
	const [openPrevModal, setOpenPrevModal] = React.useState<boolean>(false);
	const [fetchedDeviceMetadata, setFetchedDeviceMetadata] = React.useState([]);
	const [logMessage, setLogMessage] = React.useState<string>("");
	const [saveReadings, setSaveReadings] = React.useState<boolean>(false);
	const [changedSaveStaus, setChangedSaveStatus] =
		React.useState<boolean>(false);
	const [ecgResult, setECGResult] = React.useState<string>("");

	const initialTimestamp = saveReadings
		? differenceInSeconds(new Date(), new Date(deviceMetadata.timestamp))
		: 0;
	const [session, setSession] = React.useState(initialTimestamp);
	const [ecgChart, setECGChart] = React.useState<Chart<
		"line",
		number[],
		any
	> | null>(null);

	const canvasWidth = isMobile
		? 450
		: isTablet
		? 400
		: biggerTablet
		? 600
		: 800;

	const canvasHeight = isMobile ? 300 : biggerTablet ? 280 : 300;

	const deviceId = deviceMetadata?.device_id;
	const socketRef = useRef<WebSocket | null>(null);
	const isAdmin = userRole === "admin";

	const MAX_POINTS = 250;

	const handleSaveReadings = async (enable: boolean) => {
		const token = localStorage.getItem("cardiac_ai_access_token") || "";
		const deviceId = localStorage.getItem("cardiac_ai_device_id") || "";

		try {
			await saveDeviceReadings(deviceId, token, enable);
		} catch (error) {
			console.error("Error saving readings:", error);
		}
	};

	const toggleSocketConnection = (open: boolean) => {
		const socket = socketRef.current;
		if (!socket) return;
		if (open) {
			if (
				socket.readyState !== WebSocket.OPEN &&
				socket.readyState !== WebSocket.CONNECTING
			) {
				// Reconnect by creating a new socket
				socketRef.current = new WebSocket(
					`wss://api.cardiacai.tech/api/ws/frontend?device_id=${deviceId}`
				);
			}
		} else {
			if (socket.readyState === WebSocket.OPEN) {
				socket.close();
			}
		}
	};
	const formatElapsedTime = (totalSeconds: number): string => {
		const hours = String(Math.floor(totalSeconds / 3600)).padStart(2, "0");
		const minutes = String(Math.floor((totalSeconds % 3600) / 60)).padStart(
			2,
			"0"
		);
		const seconds = String(totalSeconds % 60).padStart(2, "0");
		return `${hours}:${minutes}:${seconds}`;
	};

	const downloadCurrentReadings = async () => {
		if (canvasRef.current) {
			const dataURL = canvasRef.current.toDataURL("image/png");
			const timestamp = format(new Date(), "dd-MM-yyyy:HH:mm:ss");
			const link = document.createElement("a");
			link.href = dataURL;
			link.download = `CardiacAI_Readings_${timestamp}.png`;
			link.click();
		}
	};

	const interpreteECGResult = (
		result: {
			predicted_class: ECGClasses;
			probabilities: Record<ECGClasses, number>;
		},
		threshold: number = 0.05
	) => {
		const classDescriptions: Record<ECGClasses, string> = {
			F: "Fusion of ventricular and normal beat - irregular beat caused by overlap of ventricular and normal signals.",
			M: "Fusion of paced and normal beat - mixture of pacemaker and natural heart rhythm signals.",
			N: "Normal beat - ECG appears within normal rhythm and waveform limits.",
			Q: "Unclassifiable beat - ECG pattern does not fit recognized categories.",
			S: "Supraventricular premature beat - early beat originating above the ventricles.",
			V: "Premature ventricular contraction (PVC) - extra beat originating from the ventricles.",
		};

		const recommendations: Record<ECGClasses, string> = {
			N: "Your ECG appears normal. Maintain a healthy lifestyle and routine check-ups.",
			F: "Irregular fusion beats detected. Consider consulting a cardiologist for further evaluation.",
			M: "Fusion of paced and natural beats detected. If you have a pacemaker, ensure regular follow-ups.",
			Q: "The ECG beat could not be classified. A repeat ECG or clinical review is recommended.",
			S: "Supraventricular premature beats detected. May require monitoring or lifestyle adjustments.",
			V: "Premature ventricular contractions detected. Further cardiac evaluation is advised.",
		};

		const { predicted_class: predictedClass, probabilities } = result;
		const predictedClassDescription =
			classDescriptions[predictedClass as keyof typeof classDescriptions] ||
			"Unknown class";

		console.log("Predicted Class:", predictedClassDescription);

		const sortedProbs = (
			Object.entries(probabilities) as [ECGClasses, number][]
		).sort((a, b) => b[1] - a[1]);

		const predictedProb = probabilities[predictedClass];

		// Pick alternatives within threshold margin (max 2)
		const alternatives = sortedProbs
			.filter(
				([cls, prob]) =>
					cls !== predictedClass && predictedProb - prob <= threshold
			)
			.slice(0, 2);

		// Build output text
		let output = `Predicted Class: ${predictedClass} (${(
			predictedProb * 100
		).toFixed(2)}%)\n`;
		output += `Description: ${classDescriptions[predictedClass]}\n`;

		if (alternatives.length > 0) {
			output += `\nPossible Alternatives:\n`;
			for (const [cls, prob] of alternatives) {
				output += `- ${cls} (${(prob * 100).toFixed(2)}%): ${
					classDescriptions[cls]
				}\n`;
			}
		}

		output += `\nRecommendation: ${recommendations[predictedClass]}`;

		return output;
	};

	const handleAIAnalysis = async () => {
		setLoading(true);
		try {
			const response = await requestAIAnalysis(canvasRef);

			console.log("AI Analysis Response:", response);

			setECGResult(interpreteECGResult(response));
		} catch (error) {
			console.error("Error requesting AI analysis:", error);
		} finally {
			setLoading(false);
		}
	};

	React.useEffect(() => {
		if (!deviceId) return;
		socketRef.current = new WebSocket(
			`wss://api.cardiacai.tech/api/ws/frontend?device_id=${deviceId}`
		);

		socketRef.current.onopen = () => {
			setLogMessage("Connected to backend\n");
		};

		socketRef.current.onmessage = (event) => {
			const rawValue = event.data.trim();
			const ecgPoint = parseFloat(rawValue);

			if (!isNaN(ecgPoint)) {
				const dataSet = (ecgChart?.data.datasets[0].data as number[]) || [];
				dataSet.push(ecgPoint);
				if (dataSet.length > MAX_POINTS) {
					dataSet.shift();
				}
				ecgChart?.update("none");
			} else {
				console.warn("Invalid ECG data:", rawValue);
			}
		};

		socketRef.current.onclose = () => {
			setLogMessage("Connection closed");
		};

		return () => {
			if (socketRef.current) {
				socketRef.current.close();
			}
		};
		// eslint-disable-next-line react-hooks/exhaustive-deps
	}, [deviceId, ecgChart, socketRef.current]);

	React.useEffect(() => {
		(async () => {
			if (!changedSaveStaus) return;

			await handleSaveReadings(saveReadings);
			setChangedSaveStatus(false);
		})();
	}, [saveReadings, changedSaveStaus]);

	React.useEffect(() => {
		const save = deviceMetadata?.save_status;
		setSaveReadings(save);
	}, [deviceMetadata]);

	React.useEffect(() => {
		if (!saveReadings) return;

		const timestamp = deviceMetadata.timestamp;

		const interval = setInterval(() => {
			setSession(differenceInSeconds(new Date(), new Date(timestamp)));
		}, 1000);

		return () => clearInterval(interval);
	}, [deviceMetadata, saveReadings]);

	React.useEffect(() => {
		(async () => {
			const token = localStorage.getItem("cardiac_ai_access_token") || "";
			if (openPrevModal) {
				const deviceMetadata = await getDeviceMetadata(deviceId, token);
				setFetchedDeviceMetadata(deviceMetadata);
			}
		})();
	}, [openPrevModal, deviceId]);

	React.useEffect(() => {
		if (!canvasRef.current) return;

		const ctx = canvasRef.current.getContext("2d");
		if (!ctx) return;

		if (chartRef.current) {
			chartRef.current.destroy();
		}

		const chart = new Chart(ctx, {
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
						min: 0,
						max: 4095,
						title: { display: true, text: "Amplitude" },
					},
				},
			},
		});
		setECGChart(chart);

		chartRef.current = chart;
		return () => {
			if (chartRef.current) {
				chartRef.current.destroy();
				chartRef.current = null;
			}
		};
	}, []);

	return (
		<Box
			sx={{
				width: "100vw",
				// height: "calc(100vh - 4rem)",
				height: "100vh",
				display: "flex",
				flexDirection: "column",
				justifyContent: "center",
				alignItems: "center",
				background: "rgba(249, 250, 255, 1)",
			}}
		>
			<Box
				sx={{
					background: "rgba(249, 250, 255, 1)",
					// width: { xs: "80%", sm: "70%" },
					// height: "70%",
					width: "calc(100% - 6rem)",
					height: "calc(100% - 6rem)",
					// height: '100%',
					display: "flex",
					flexDirection: "column",
					padding: { xs: "3rem 1.5rem", md: "3rem" },
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
					{isAdmin ? (
						<FormControlLabel
							control={
								<Switch
									checked={saveReadings}
									sx={{ zIndex: 1 }}
									onClick={() => {
										setSaveReadings((prev) => !prev);
										setChangedSaveStatus(true);
									}}
								/>
							}
							label="Save Data"
						/>
					) : (
						""
					)}

					<Typography>
						Device ID: <b>{deviceId}</b>
					</Typography>
					<Typography>
						Session: <b>{formatElapsedTime(session)}</b>
					</Typography>
				</Box>
				<Box sx={{ width: "100%", margin: "1rem 0" }}>
					<canvas
						id="ecgChart"
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
				<Box>
					<Button
						variant="contained"
						sx={{
							border: "1px solid rgba(47, 90, 196, 1)",
							background: "rgba(47, 90, 196, 1)",
							padding: "0.5rem 1rem",
							cursor: loading ? "not-allowed" : "pointer",
						}}
						onClick={() => {
							setOpenSocket((prev) => {
								toggleSocketConnection(!prev);
								return !prev;
							});
						}}
					>
						{openSocket ? "Pause" : "Resume"}
					</Button>
				</Box>
				{isAdmin ? (
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
								cursor: loading ? "not-allowed" : "pointer",

								":hover": {
									border: "2px solid rgba(47, 90, 196, 1)",
									background: "rgba(47, 90, 196, 1)",
									color: "#fff",
								},
							}}
							onClick={() => setOpenPrevModal(true)}
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
								cursor: loading ? "not-allowed" : "pointer",

								":hover": {
									border: "2px solid rgba(47, 90, 196, 1)",
									background: "rgba(47, 90, 196, 1)",
									color: "#fff",
								},
							}}
							onClick={downloadCurrentReadings}
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
								cursor: loading ? "not-allowed" : "pointer",

								":hover": {
									border: "2px solid rgba(47, 90, 196, 1)",
									background: "rgba(47, 90, 196, 1)",
									color: "#fff",
								},
							}}
							onClick={handleAIAnalysis}
						>
							Request AI Analysis {"   "}
							{loading ? <CircularProgress size={"1rem"} /> : ""}
						</Button>
					</Box>
				) : (
					""
				)}

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
						disabled
						value={ecgResult}
					/>
				</Box>
				<PrevReadingModal
					open={openPrevModal}
					onClose={() => setOpenPrevModal(false)}
					data={fetchedDeviceMetadata}
				/>
			</Box>
		</Box>
	);
};

export default Analysis;
