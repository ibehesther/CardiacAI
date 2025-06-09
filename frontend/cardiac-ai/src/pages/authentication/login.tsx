import { Alert, Box, Button, Collapse, Input, Typography } from "@mui/material";
import React, { useRef } from "react";
import { useAuth } from "../../context/AuthProvider";
import { useNavigate } from "react-router-dom";

const Login = () => {
	const inputRef = useRef<{ [key: string]: string }>({});
	const { login } = useAuth();
	const navigate = useNavigate()

	const [formData, setFormData] = React.useState({
		deviceId: "",
		password: "",
	});
	const [alertData, setAlertData] = React.useState<{
		open: boolean;
		type: "error" | "success";
		message: string;
	}>({
		open: false,
		type: "success",
		message: "",
	});
	const [loading, setLoading] = React.useState(false);

	const handleChange = (e: React.ChangeEvent<HTMLInputElement>) => {
		const { id, value } = e.target;
		inputRef.current.id = id;
		inputRef.current.value = value;

		setFormData((prev) => ({ ...prev, [id]: value }));
	};

	const handleSubmit = async (e: React.FormEvent) => {
		e.preventDefault();
		setLoading(true);
		try {
			await login(formData.deviceId, formData.password);
			setAlertData({
				open: true,
				type: "success",
				message: "Login successful!",
			});
			navigate("/analysis");
		} catch (error) {
			console.error("An error occured: ", error);
			setAlertData({
				open: true,
				type: "error",
				message: "Login failed. Please check your device ID and password.",
			});
		} finally {
			setLoading(false);
			setFormData({ deviceId: "", password: "" });
			setTimeout(() => {
				setAlertData((prev) => ({ ...prev, open: false }));
			}, 3000);
		}
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
				background:
					'rgba(249, 250, 255, 1) url("/images/heart_rate.png") no-repeat fixed bottom',
				backgroundSize: "contain",
			}}
		>
			<Collapse
				in={alertData.open}
				sx={{ position: "fixed", width: "80%", top: "7rem" }}
				timeout={{ exit: 500, enter: 500 }}
			>
				<Alert
					severity={alertData.type}
					onClose={() => {
						setAlertData((prev) => ({ ...prev, open: false }));
					}}
				>
					{alertData.message}
				</Alert>
			</Collapse>

			<Typography
				sx={{
					fontSize: { xs: "1.875rem", sm: "3rem", md: "3.5rem" },
					fontWeight: 900,
					color: "rgba(18, 56, 144, 1)",
					lineHeight: { xs: "2rem", sm: "3rem", md: "4rem" },
				}}
			>
				Get Started
			</Typography>
			<Box
				component="form"
				onSubmit={handleSubmit}
				sx={{
					marginTop: "10rem",
					display: "flex",
					flexDirection: "column",
					gap: "2rem",
					width: { xs: "80%", md: "60%", lg: "50%" },
				}}
			>
				<Input
					placeholder="Input device ID"
					id="deviceId"
					value={formData.deviceId}
					onChange={handleChange}
					autoFocus={inputRef.current["id"] === "deviceId"}
					sx={{
						border: "2px solid rgba(0, 0, 0, 0.4)",
						padding: "1rem",
						fontSize: { xs: "1.25rem", sm: "1.5rem" },
						borderRadius: "1rem",
						":focus": {
							border: "2px solid rgba(61, 119, 242, 1)",
						},
						"::after": {
							border: 0,
						},
						"::before": {
							borderBottom: "0 !important",
						},
					}}
				/>
				<Input
					placeholder="Password"
					id="password"
					type="password"
					value={formData.password}
					onChange={handleChange}
					autoFocus={inputRef.current["id"] === "password"}
					sx={{
						border: "2px solid rgba(0, 0, 0, 0.4)",
						padding: "1rem",
						fontSize: { xs: "1.25rem", sm: "1.5rem" },
						borderRadius: "1rem",
						":focus": {
							border: "2px solid rgba(61, 119, 242, 1)",
						},
						"::after": {
							border: 0,
						},
						"::before": {
							borderBottom: "0 !important",
						},
					}}
				/>
				<Button
					variant="contained"
					sx={{
						backgroundColor: "rgba(61, 119, 242, 1)",
						padding: { xs: "1rem", sm: "1rem 1.5rem" },
						borderRadius: "1.25rem",
						fontSize: { xs: "1.25rem", sm: "1.5rem" },
					}}
					type="submit"
					disabled={loading}
				>
					Get Started
				</Button>
				<Button
					variant="text"
					sx={{
						color: "rgba(61, 119, 242, 1)",
						fontSize: { xs: "1rem", sm: "1.25rem" },
						textTransform: "capitalize",
					}}
				>
					Forgotten device ID?
				</Button>
			</Box>
		</Box>
	);
};

export default Login;
