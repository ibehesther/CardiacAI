import axios from "axios";
import { format } from "date-fns/format";

const REACT_APP_API_URL = process.env.REACT_APP_API_URL;

export const saveDeviceReadings = async (
	deviceId: string,
	token: string,
	enable: boolean
) => {
	try {
		const response = await axios.post(
			`${REACT_APP_API_URL}/api/readings/save/${deviceId}?enable=${enable}`,
			{},
			{
				headers: {
					"Content-Type": "application/json",
					"Access-Control-Allow-Origin": "*",
					Authorization: `Bearer ${token}`,
				},
			}
		);

		console.log("Token Used: ", token);

		console.log("Saving device readings for token:", response);
		return response.data;
	} catch (error) {
		console.error("Error saving device readings:", error);
	}
};

export const downloadDeviceReadings = async (
	sessionId: string,
	token: string
) => {
	const response = await axios.get(
		`${REACT_APP_API_URL}/api/readings/download/${sessionId}`,
		{
			responseType: "blob",
			headers: {
				"Content-Type": "application/json",
				"Access-Control-Allow-Origin": "*",
				Authorization: `Bearer ${token}`,
			},
		}
	);

	return response.data;
};

export const requestAIAnalysis = async (canvasRef: React.RefObject<HTMLCanvasElement | null>) => {
		if (canvasRef.current) {
			const dataURL = canvasRef.current.toDataURL("image/png");
			const res = await fetch(dataURL);
			const blob = await res.blob();

			const timestamp = format(new Date(), "dd-MM-yyyy:HH:mm:ss");
			const file = new File([blob], `CardiacAI_Readings_${timestamp}.png`, {
				type: "image/png",
			});

			const formData = new FormData();
			formData.append("file", file);

			try {
				const response = await axios.post(
					`${REACT_APP_API_URL}/ai/predict`,
					formData,
					{
						headers: {
							"Content-Type": "multipart/form-data",
							"Access-Control-Allow-Origin": "*",
						},
					}
				);

				return response.data
			} catch (error) {
				console.error("Error requesting AI analysis:", error);
			}
		}
	};


